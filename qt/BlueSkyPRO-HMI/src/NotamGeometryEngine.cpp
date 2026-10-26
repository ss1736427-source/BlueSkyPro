#include "NotamGeometryEngine.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QtMath>

namespace {
struct Coord { double lat=0, lon=0; bool ok=false; };
Coord coord(const QString &s) {
    static const QRegularExpression re(QStringLiteral("(\\d{2})(\\d{2})(\\d{2})?([NS])\\s*(\\d{3})(\\d{2})(\\d{2})?([EW])"), QRegularExpression::CaseInsensitiveOption);
    auto m=re.match(s); if(!m.hasMatch()) return {};
    double a=m.captured(1).toDouble()+m.captured(2).toDouble()/60.0+m.captured(3).toDouble()/3600.0;
    double b=m.captured(5).toDouble()+m.captured(6).toDouble()/60.0+m.captured(7).toDouble()/3600.0;
    if(a>90||b>180)return {};
    if(m.captured(4).compare("S",Qt::CaseInsensitive)==0)a=-a;
    if(m.captured(8).compare("W",Qt::CaseInsensitive)==0)b=-b;
    return {a,b,true};
}
bool alt(QString s,double &m,QString &datum) {
    s=s.trimmed().toUpper();
    if(s=="GND"||s=="SFC"){m=0;datum="AGL";return true;}
    static const QRegularExpression fl(QStringLiteral("^FL\\s*(\\d{2,3})$"));
    auto f=fl.match(s);if(f.hasMatch()){m=f.captured(1).toDouble()*100*0.3048;datum="FL";return true;}
    static const QRegularExpression n(QStringLiteral("^(\\d+(?:[.,]\\d+)?)\\s*(FT|M)\\s*(AGL|AMSL|MSL)$"));
    auto x=n.match(s);if(!x.hasMatch())return false;
    m=x.captured(1).replace(',','.').toDouble()*(x.captured(2)=="FT"?0.3048:1.0);
    datum=x.captured(3)=="MSL"?"AMSL":x.captured(3);return true;
}
QJsonArray circle(double lat,double lon,double km) {
    QJsonArray ring;const double rad=qDegreesToRadians(lat);constexpr double pi=3.14159265358979323846;
    for(int i=0;i<72;++i){double a=2*pi*i/72;ring.append(QJsonArray{lon+km*qCos(a)/(111.32*qMax(0.01,qCos(rad))),lat+km*qSin(a)/110.574});}
    ring.append(ring.first());return ring;
}
}
QVariantMap NotamGeometryEngine::conservativeUpperPlane(double minTerrainMeters,
                                                                      double maxTerrainMeters,
                                                                      double totalRestrictionHeightMeters) {
    QVariantMap out;
    out["status"] = "INVALID_TERRAIN_OR_HEIGHT";
    out["valid"] = false;
    if (!qIsFinite(minTerrainMeters) || !qIsFinite(maxTerrainMeters) ||
        !qIsFinite(totalRestrictionHeightMeters) ||
        maxTerrainMeters < minTerrainMeters || totalRestrictionHeightMeters < 0.0) {
        return out;
    }

    // The restriction top is a constant horizontal plane. Terrain variation
    // must not raise or tilt the NOTAM ceiling; the caller supplies the source
    // NOTAM height in metres in a consistent vertical reference.
    const double upperPlane = minTerrainMeters + totalRestrictionHeightMeters;
    out["minimumTerrainMeters"] = minTerrainMeters;
    out["maximumTerrainMeters"] = maxTerrainMeters;
    out["sourceUpperPlaneMeters"] = upperPlane;
    out["upperPlaneMeters"] = upperPlane;
    out["adjustedToTerrainPeak"] = false;
    out["valid"] = true;
    out["status"] = "FLAT_SOURCE_HEIGHT_PLANE";
    return out;
}

QVariantMap NotamGeometryEngine::normalize(const QVariantMap &source) {
    QVariantMap out=source;
    const QString text=source.value("originalText").toString()+"\n"+source.value("text").toString();
    QString lower=source.value("lowerLimitRaw").toString().trimmed(),upper=source.value("upperLimitRaw").toString().trimmed();
    static const QRegularExpression fr(QStringLiteral("(?:^|\\n)F\\)\\s*([^\\r\\n]+)"),QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression gr(QStringLiteral("(?:^|\\n)G\\)\\s*([^\\r\\n]+)"),QRegularExpression::CaseInsensitiveOption);
    if(lower.isEmpty()){auto m=fr.match(text);if(m.hasMatch())lower=m.captured(1).trimmed().split(QRegularExpression("\\s{2,}")).first();}
    if(upper.isEmpty()){auto m=gr.match(text);if(m.hasMatch())upper=m.captured(1).trimmed().split(QRegularExpression("\\s{2,}")).first();}
    double lo=0,hi=0;QString ld,hd;bool vok=alt(lower,lo,ld)&&alt(upper,hi,hd)&&hi>lo&&ld==hd;
    out["lowerLimitRaw"]=lower;out["upperLimitRaw"]=upper;out["lowerMeters"]=lo;out["upperMeters"]=hi;
    out["lowerDatum"]=ld;out["upperDatum"]=hd;
    QJsonObject g;const QString raw=source.value("geometryJson").toString();
    auto gd=QJsonDocument::fromJson(raw.toUtf8());if(gd.isObject())g=gd.object();
    bool compoundParsed=false;
    if(g.value("type").toString()!="Polygon"&&g.value("type").toString()!="MultiPolygon"){
        // Parse numbered "AS FLW" components independently, so circle centres
        // cannot accidentally become vertices of the preceding area boundary.
        static const QRegularExpression sectionRe(QStringLiteral("AS\\s+FLW\\s*:\\s*([\\s\\S]*?)(?=\\bF\\)|\\bG\\)|$)"),QRegularExpression::CaseInsensitiveOption);
        const auto sectionMatch=sectionRe.match(text);
        if(sectionMatch.hasMatch()){
            const QString section=sectionMatch.captured(1);
            static const QRegularExpression itemRe(QStringLiteral("(?:^|\\n)\\s*\\d+\\.\\s*([\\s\\S]*?)(?=(?:\\n\\s*\\d+\\.)|$)"));
            static const QRegularExpression coordinateRe(QStringLiteral("\\d{4,6}[NS]\\s*\\d{5,7}[EW]"),QRegularExpression::CaseInsensitiveOption);
            static const QRegularExpression circleRe(QStringLiteral("RADIUS\\s*(\\d+(?:[.,]\\d+)?)\\s*(NM|KM|M)\\s+CENT(?:RE|ER)\\s+(\\d{4,6}[NS]\\s*\\d{5,7}[EW])"),QRegularExpression::CaseInsensitiveOption);
            QJsonArray polygons;
            auto items=itemRe.globalMatch(section);
            bool allItemsValid=true;
            int itemCount=0;
            while(items.hasNext()){
                const QString item=items.next().captured(1);
                QJsonArray ring;
                const auto circleMatch=circleRe.match(item);
                if(circleMatch.hasMatch()){
                    QString radiusText=circleMatch.captured(1);radiusText.replace(',','.');
                    double radiusKm=radiusText.toDouble();
                    const QString unit=circleMatch.captured(2).toUpper();
                    if(unit=="NM")radiusKm*=1.852;else if(unit=="M")radiusKm/=1000.0;
                    const Coord centre=coord(circleMatch.captured(3));
                    if(!centre.ok||radiusKm<=0||radiusKm>500){allItemsValid=false;break;}
                    ring=circle(centre.lat,centre.lon,radiusKm);
                }else{
                    auto coordinates=coordinateRe.globalMatch(item);
                    while(coordinates.hasNext()){
                        const Coord point=coord(coordinates.next().captured());
                        if(point.ok)ring.append(QJsonArray{point.lon,point.lat});
                    }
                    if(ring.size()<3){allItemsValid=false;break;}
                    if(ring.first()!=ring.last())ring.append(ring.first());
                }
                polygons.append(QJsonArray{ring});
                ++itemCount;
            }
            if(allItemsValid&&itemCount>=2&&polygons.size()==itemCount){
                g=QJsonObject{{"type","MultiPolygon"},{"coordinates",polygons}};
                compoundParsed=true;
            }
        }
    }
    if(!compoundParsed&&g.value("type").toString()!="Polygon"&&g.value("type").toString()!="MultiPolygon"){
        static const QRegularExpression area(QStringLiteral("(?:AIRSPACE\\s+CLSD\\s+WI\\s+AREA|AREA|DEFINED\\s+BY)\\s*:?([\\s\\S]*?)(?=\\bF\\)|\\bG\\)|$)"),QRegularExpression::CaseInsensitiveOption);
        auto am=area.match(text);QString candidate=am.hasMatch()?am.captured(1):QString();
        static const QRegularExpression cr(QStringLiteral("\\d{4,6}[NS]\\s*\\d{5,7}[EW]"),QRegularExpression::CaseInsensitiveOption);
        QJsonArray ring;auto it=cr.globalMatch(candidate);
        while(it.hasNext()){auto c=coord(it.next().captured());if(c.ok)ring.append(QJsonArray{c.lon,c.lat});}
        if(ring.size()>=3){if(ring.first()!=ring.last())ring.append(ring.first());g=QJsonObject{{"type","Polygon"},{"coordinates",QJsonArray{ring}}};}
        else {
            // Only explicit radius wording builds a circle; the Q-line radius is search metadata.
            static const QRegularExpression rr(QStringLiteral("(?:CIRCLE\\s+)?RADIUS\\s*[-:]?\\s*(\\d+(?:[.,]\\d+)?)\\s*(NM|KM|M)\\s+(?:CENT(?:RE|ER)|OF|AROUND)\\s+(\\d{4,6}[NS]\\s*\\d{5,7}[EW])|(?:WITHIN|WI)\\s+(\\d+(?:[.,]\\d+)?)\\s*(NM|KM|M)\\s+(?:OF|FROM|CENT(?:RE|ER)\\s+OF)\\s+(\\d{4,6}[NS]\\s*\\d{5,7}[EW])"),QRegularExpression::CaseInsensitiveOption);
            auto rm=rr.match(text);if(rm.hasMatch()){
                bool first=!rm.captured(1).isEmpty();QString n=(first?rm.captured(1):rm.captured(4));n.replace(',','.');QString unit=(first?rm.captured(2):rm.captured(5)).toUpper();
                auto c=coord(first?rm.captured(3):rm.captured(6));double km=n.toDouble();if(unit=="NM")km*=1.852;else if(unit=="M")km/=1000;
                if(c.ok&&km>0&&km<=500)g=QJsonObject{{"type","Polygon"},{"coordinates",QJsonArray{circle(c.lat,c.lon,km)}}};
            }
        }
    }
    QString type=g.value("type").toString();
    const bool areaGeometry=type=="Polygon"||type=="MultiPolygon";
    // Keep valid non-area GeoJSON for map display. Point/line geometry is useful
    // for situational awareness, but must never be reported as a validated area restriction.
    const bool displayGeometry=areaGeometry||type=="Point"||type=="MultiPoint"
        ||type=="LineString"||type=="MultiLineString";
    out["geometryJson"]=displayGeometry?QString::fromUtf8(QJsonDocument(g).toJson(QJsonDocument::Compact)):QString();
    out["geometryType"]=displayGeometry?type:QString();
    out["geometryValidated"]=areaGeometry&&vok;
    out["restrictionGeometryStatus"]=areaGeometry?(vok?"VALIDATED":"MISSING_OR_INCOMPATIBLE_VERTICAL_BOUNDS")
        :(displayGeometry?"DISPLAY_ONLY_NON_AREA_GEOMETRY":"MISSING_HORIZONTAL_BOUNDS");
    out["altitudeReference"]=vok?ld:"UNKNOWN";
    return out;
}
