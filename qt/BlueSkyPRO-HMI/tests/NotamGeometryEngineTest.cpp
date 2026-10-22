#include "../src/NotamGeometryEngine.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cassert>
int main(int argc,char**argv){
 QCoreApplication app(argc,argv);
 QVariantMap p{{"id","V7637/26"},{"originalText","E)AIRSPACE CLSD WI AREA: 433650N0393600E-433539N0394029E-433438N0394038E-433338N0394213E-433020N0393900E-433650N0393600E\nF)GND\nG)6000M AGL"},{"lowerLimitRaw","GND"},{"upperLimitRaw","6000M AGL"}};
 auto a=NotamGeometryEngine::normalize(p);assert(a["geometryValidated"].toBool());auto g=QJsonDocument::fromJson(a["geometryJson"].toString().toUtf8()).object();assert(g["type"].toString()=="Polygon");assert(g["coordinates"].toArray().first().toArray().size()==6);
 QVariantMap c{{"originalText","E)RESTRICTED AREA RADIUS 2NM CENTRE 433650N0393600E\nF)GND\nG)3000FT AGL"},{"lowerLimitRaw","GND"},{"upperLimitRaw","3000FT AGL"}};
 auto b=NotamGeometryEngine::normalize(c);assert(b["geometryValidated"].toBool());assert(QJsonDocument::fromJson(b["geometryJson"].toString().toUtf8()).object()["coordinates"].toArray().first().toArray().size()==73);
 auto plane=NotamGeometryEngine::conservativeUpperPlane(300.0,2400.0,2200.0);
 assert(plane["valid"].toBool());
 assert(qAbs(plane["upperPlaneMeters"].toDouble()-2500.0)<0.001);
 assert(!plane["adjustedToTerrainPeak"].toBool());
 auto raised=NotamGeometryEngine::conservativeUpperPlane(300.0,2400.0,2100.0);
 assert(raised["valid"].toBool());
 assert(raised["status"].toString()=="RAISED_TO_HIGHEST_TERRAIN");
 assert(raised["adjustedToTerrainPeak"].toBool());
 assert(qAbs(raised["sourceUpperPlaneMeters"].toDouble()-2400.0)<0.001);
 assert(qAbs(raised["upperPlaneMeters"].toDouble()-2400.0)<0.001);
 auto invalidTerrain=NotamGeometryEngine::conservativeUpperPlane(2500.0,2400.0,3000.0);
 assert(!invalidTerrain["valid"].toBool());
 QVariantMap bad{{"text","Informational NOTAM only"}};auto d=NotamGeometryEngine::normalize(bad);assert(!d["geometryValidated"].toBool());
 // Non-area GeoJSON must remain available to the map's point/line layers, but
 // must never be reported as a validated airspace polygon.
 QVariantMap point{{"geometryJson",QStringLiteral(R"({"type":"Point","coordinates":[39.6,43.6]})")}};
 auto pointResult=NotamGeometryEngine::normalize(point);
 assert(pointResult["geometryType"].toString()=="Point");
 assert(!pointResult["geometryJson"].toString().isEmpty());
 assert(!pointResult["geometryValidated"].toBool());
 assert(pointResult["restrictionGeometryStatus"].toString()=="DISPLAY_ONLY_NON_AREA_GEOMETRY");
 QVariantMap line{{"geometryJson",QStringLiteral(R"({"type":"LineString","coordinates":[[39.6,43.6],[39.7,43.7]]})")}};
 auto lineResult=NotamGeometryEngine::normalize(line);
 assert(lineResult["geometryType"].toString()=="LineString");
 assert(!lineResult["geometryJson"].toString().isEmpty());
 assert(!lineResult["geometryValidated"].toBool());
 assert(lineResult["restrictionGeometryStatus"].toString()=="DISPLAY_ONLY_NON_AREA_GEOMETRY");
 QVariantMap qline{{"originalText","Q)URRV/QRTCA/IV/BO/W/000/999/4334N03938E004\\nF)GND\\nG)6000M AGL"}};auto q=NotamGeometryEngine::normalize(qline);assert(q["geometryJson"].toString().isEmpty());assert(!q["geometryValidated"].toBool());return 0;
}