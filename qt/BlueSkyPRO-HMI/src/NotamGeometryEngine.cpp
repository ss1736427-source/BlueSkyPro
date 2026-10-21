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
    if(g.value("type").toString()!="Polygon"&&g.value("type").toString()!="MultiPolygon"){
        static const QRegularExpression area(QStringLiteral("(?:AIRSPACE\\s+CLSD\\s+WI\\s+AREA|AREA|DEFINED\\s+BY)\\s*:?([\\s\\S]*?)(?=\\bF\\)|\\bG\\)|$)"),QRegularExpression::CaseInsensitiveOption);
        auto am=area.match(text);QString candidate=am.hasMatch()?am.captured(1):text;
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
    QString type=g.value("type").toString();bool hok=type=="Polygon"||type=="MultiPolygon";
    out["geometryJson"]=hok?QString::fromUtf8(QJsonDocument(g).toJson(QJsonDocument::Compact)):QString();
    out["geometryType"]=hok?type:QString();
    out["geometryValidated"]=hok&&vok;
    out["restrictionGeometryStatus"]=!hok?"MISSING_HORIZONTAL_BOUNDS":!vok?"MISSING_OR_INCOMPATIBLE_VERTICAL_BOUNDS":"VALIDATED";
    out["altitudeReference"]=vok?ld:"UNKNOWN";
    return out;
}
