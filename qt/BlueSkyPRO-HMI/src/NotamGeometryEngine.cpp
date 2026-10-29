#include "NotamGeometryEngine.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QVector>
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
struct GeoPoint { double lon=0.0, lat=0.0; };

bool toGeoPoint(const QJsonValue &value, GeoPoint &point) {
    const QJsonArray coordinate = value.toArray();
    if (coordinate.size() < 2) return false;
    point = {coordinate.at(0).toDouble(), coordinate.at(1).toDouble()};
    return qIsFinite(point.lon) && qIsFinite(point.lat)
        && point.lon >= -180.0 && point.lon <= 180.0
        && point.lat >= -90.0 && point.lat <= 90.0;
}

bool samePoint(const GeoPoint &a, const GeoPoint &b) {
    return qAbs(a.lon-b.lon) < 1e-9 && qAbs(a.lat-b.lat) < 1e-9;
}

QJsonArray closedValidRing(QJsonArray ring) {
    if (ring.size() < 3) return {};
    GeoPoint first, last;
    for (const QJsonValue &value : ring) {
        GeoPoint point;
        if (!toGeoPoint(value, point)) return {};
    }
    if (!toGeoPoint(ring.first(), first) || !toGeoPoint(ring.last(), last)) return {};
    if (!samePoint(first, last)) ring.append(ring.first());
    return ring.size() >= 4 ? ring : QJsonArray{};
}

double orientation(const GeoPoint &a, const GeoPoint &b, const GeoPoint &c) {
    return (b.lon-a.lon)*(c.lat-a.lat)-(b.lat-a.lat)*(c.lon-a.lon);
}

bool onSegment(const GeoPoint &a, const GeoPoint &b, const GeoPoint &p) {
    return p.lon >= qMin(a.lon,b.lon)-1e-10 && p.lon <= qMax(a.lon,b.lon)+1e-10
        && p.lat >= qMin(a.lat,b.lat)-1e-10 && p.lat <= qMax(a.lat,b.lat)+1e-10;
}

bool segmentsIntersect(const GeoPoint &a, const GeoPoint &b, const GeoPoint &c, const GeoPoint &d) {
    const double abC=orientation(a,b,c), abD=orientation(a,b,d);
    const double cdA=orientation(c,d,a), cdB=orientation(c,d,b);
    const auto opposite=[](double left,double right) { return (left>1e-10&&right< -1e-10)||(left< -1e-10&&right>1e-10); };
    if (opposite(abC,abD) && opposite(cdA,cdB)) return true;
    if (qAbs(abC)<=1e-10 && onSegment(a,b,c)) return true;
    if (qAbs(abD)<=1e-10 && onSegment(a,b,d)) return true;
    if (qAbs(cdA)<=1e-10 && onSegment(c,d,a)) return true;
    if (qAbs(cdB)<=1e-10 && onSegment(c,d,b)) return true;
    return false;
}

bool ringSelfIntersects(const QJsonArray &ring) {
    if (ring.size() < 4) return true;
    QVector<GeoPoint> points;
    points.reserve(ring.size());
    for (const QJsonValue &value : ring) {
        GeoPoint point;
        if (!toGeoPoint(value, point)) return true;
        points.append(point);
    }
    const int segmentCount=points.size()-1;
    for (int i=0;i<segmentCount;++i) {
        for (int j=i+1;j<segmentCount;++j) {
            if (j==i+1 || (i==0 && j==segmentCount-1)) continue;
            if (segmentsIntersect(points.at(i),points.at(i+1),points.at(j),points.at(j+1))) return true;
        }
    }
    return false;
}

bool pointInRing(const GeoPoint &point, const QJsonArray &ring) {
    bool inside=false;
    if (ring.size() < 4) return false;
    for (int i=0,j=ring.size()-2;i<ring.size()-1;j=i++) {
        GeoPoint a,b;
        if (!toGeoPoint(ring.at(i),a) || !toGeoPoint(ring.at(j),b)) return false;
        const bool crosses=(a.lat>point.lat)!=(b.lat>point.lat)
            && point.lon < (b.lon-a.lon)*(point.lat-a.lat)/(b.lat-a.lat+1e-30)+a.lon;
        if (crosses) inside=!inside;
    }
    return inside;
}

QJsonObject sanitiseRawAreaGeometry(const QJsonObject &geometry) {
    const QString type=geometry.value("type").toString();
    if (type != QStringLiteral("Polygon") && type != QStringLiteral("MultiPolygon")) return geometry;
    const QJsonArray sourcePolygons=type == QStringLiteral("Polygon")
        ? QJsonArray{geometry.value("coordinates").toArray()}
        : geometry.value("coordinates").toArray();
    QJsonArray outputPolygons;
    for (const QJsonValue &polygonValue : sourcePolygons) {
        const QJsonArray sourceRings=polygonValue.toArray();
        if (sourceRings.isEmpty()) return {};
        QJsonArray rings;
        for (const QJsonValue &ringValue : sourceRings) {
            const QJsonArray ring=closedValidRing(ringValue.toArray());
            if (ring.isEmpty() || ringSelfIntersects(ring)) return {};
            rings.append(ring);
        }
        GeoPoint outerPoint;
        if (!toGeoPoint(rings.first().toArray().first(), outerPoint)) return {};
        bool independentRings=false;
        for (int i=1;i<rings.size();++i) {
            GeoPoint innerPoint;
            if (!toGeoPoint(rings.at(i).toArray().first(), innerPoint)) return {};
            if (!pointInRing(innerPoint, rings.first().toArray())) { independentRings=true; break; }
        }
        if (independentRings) {
            for (const QJsonValue &ringValue : rings) outputPolygons.append(QJsonArray{ringValue});
        } else {
            outputPolygons.append(rings);
        }
    }
    if (outputPolygons.isEmpty()) return {};
    return outputPolygons.size()==1
        ? QJsonObject{{"type", "Polygon"}, {"coordinates", outputPolygons.first().toArray()}}
        : QJsonObject{{"type", "MultiPolygon"}, {"coordinates", outputPolygons}};
}

QString ringSignature(const QJsonArray &ring) {
    QStringList parts;
    parts.reserve(ring.size());
    for (const QJsonValue &value : ring) {
        const QJsonArray point = value.toArray();
        if (point.size() < 2) return {};
        parts.append(QString::number(point.at(0).toDouble(), 'f', 8)
                     + QLatin1Char(',')
                     + QString::number(point.at(1).toDouble(), 'f', 8));
    }
    return parts.join(QLatin1Char('|'));
}

void appendUniquePolygon(QJsonArray &polygons, QSet<QString> &seen, QJsonArray ring) {
    ring=closedValidRing(ring);
    if (ring.isEmpty() || ringSelfIntersects(ring)) return;
    const QString signature = ringSignature(ring);
    if (signature.isEmpty() || seen.contains(signature)) return;
    seen.insert(signature);
    polygons.append(QJsonArray{ring});
}

QJsonArray explicitRestrictionPolygons(const QString &text) {
    QJsonArray polygons;
    QSet<QString> seen;
    static const QRegularExpression coordinate(
        QStringLiteral("\\d{4,6}[NS]\\s*\\d{5,7}[EW]"),
        QRegularExpression::CaseInsensitiveOption);
    // An AREA stops before the next enumerated area/circle, F)/G), or a repeated
    // AIRSPACE heading. This prevents circle centres from becoming polygon vertices.
    static const QRegularExpression areaBlock(
        QStringLiteral("\\bAREA\\s*:\\s*([\\s\\S]*?)(?=(?:\\r?\\n\\s*\\d+\\.\\s*(?:(?:WI\\s+)?CIRCLE|AREA)\\b)|(?:\\r?\\n\\s*[FG]\\))|(?:\\r?\\n\\s*AIRSPACE\\s+CLSD\\b)|$)"),
        QRegularExpression::CaseInsensitiveOption);
    auto areas = areaBlock.globalMatch(text);
    while (areas.hasNext()) {
        const QString candidate = areas.next().captured(1);
        QJsonArray ring;
        auto coordinates = coordinate.globalMatch(candidate);
        while (coordinates.hasNext()) {
            const Coord parsed = coord(coordinates.next().captured());
            if (parsed.ok) ring.append(QJsonArray{parsed.lon, parsed.lat});
        }
        appendUniquePolygon(polygons, seen, ring);
    }

    // A NOTAM may define several circles after an AREA. Every explicit circle is
    // independently rendered; Q-line radius remains only search metadata.
    static const QRegularExpression circleBlock(
        QStringLiteral("(?:\\bWI\\s+)?(?:CIRCLE\\s+)?RADIUS\\s*[-:]?\\s*(\\d+(?:[.,]\\d+)?)\\s*(NM|KM|M)\\s+(?:CENT(?:RE|ER)(?:\\s+OF)?|OF|AROUND)\\s+(\\d{4,6}[NS]\\s*\\d{5,7}[EW])|\\b(?:WI|WITHIN)\\s+(\\d+(?:[.,]\\d+)?)\\s*(NM|KM|M)\\s+(?:OF|FROM|CENT(?:RE|ER)\\s+OF)\\s+(\\d{4,6}[NS]\\s*\\d{5,7}[EW])"),
        QRegularExpression::CaseInsensitiveOption);
    auto circles = circleBlock.globalMatch(text);
    while (circles.hasNext()) {
        const QRegularExpressionMatch match = circles.next();
        const bool radiusFirst = !match.captured(1).isEmpty();
        QString radiusText = radiusFirst ? match.captured(1) : match.captured(4);
        radiusText.replace(QLatin1Char(','), QLatin1Char('.'));
        const QString unit = (radiusFirst ? match.captured(2) : match.captured(5)).toUpper();
        const Coord centre = coord(radiusFirst ? match.captured(3) : match.captured(6));
        double kilometres = radiusText.toDouble();
        if (unit == QStringLiteral("NM")) kilometres *= 1.852;
        else if (unit == QStringLiteral("M")) kilometres /= 1000.0;
        if (centre.ok && kilometres > 0.0 && kilometres <= 500.0)
            appendUniquePolygon(polygons, seen, circle(centre.lat, centre.lon, kilometres));
    }
    return polygons;
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
    // Prefer explicit compound text over an upstream single-polygon approximation.
    {
        // Parse every AS FLW section. Some NOTAMs repeat the same numbered
        // geometry block after F)/G); identical rings are deduplicated below.
        static const QRegularExpression sectionRe(QStringLiteral("AS\\s+FLW\\s*:\\s*([\\s\\S]*?)(?=\\bF\\)|\\bG\\)|$)"),QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression itemRe(QStringLiteral("(?:^|\\n)\\s*\\d+\\.\\s*([\\s\\S]*?)(?=(?:\\n\\s*\\d+\\.)|$)"));
        static const QRegularExpression coordinateRe(QStringLiteral("\\d{4,6}[NS]\\s*\\d{5,7}[EW]"),QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression circleRe(QStringLiteral("RADIUS\\s*(\\d+(?:[.,]\\d+)?)\\s*(NM|KM|M)\\s+CENT(?:RE|ER)\\s+(\\d{4,6}[NS]\\s*\\d{5,7}[EW])"),QRegularExpression::CaseInsensitiveOption);
        QJsonArray polygons;
        QSet<QString> seenRings;
        auto sections=sectionRe.globalMatch(text);
        bool allItemsValid=true;
        int itemCount=0;
        while(sections.hasNext()&&allItemsValid){
            const QString section=sections.next().captured(1);
            auto items=itemRe.globalMatch(section);
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
                // Repeated NOTAM text must not duplicate an area or circle.
                const QString ringKey=QString::fromUtf8(QJsonDocument(QJsonArray{ring}).toJson(QJsonDocument::Compact));
                if(seenRings.contains(ringKey)) continue;
                seenRings.insert(ringKey);
                polygons.append(QJsonArray{ring});
                ++itemCount;
            }
        }
        if(allItemsValid&&itemCount>=2&&polygons.size()==itemCount){
            g=QJsonObject{{"type","MultiPolygon"},{"coordinates",polygons}};
            compoundParsed=true;
        }
    }
    if (!compoundParsed) {
        const QJsonArray textualPolygons = explicitRestrictionPolygons(text);
        if (!textualPolygons.isEmpty()) {
            g = textualPolygons.size() == 1
                ? QJsonObject{{"type", "Polygon"}, {"coordinates", textualPolygons.first()}}
                : QJsonObject{{"type", "MultiPolygon"}, {"coordinates", textualPolygons}};
            compoundParsed = true;
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
    g = sanitiseRawAreaGeometry(g);
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
