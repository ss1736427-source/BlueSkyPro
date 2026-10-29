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
 auto flat=NotamGeometryEngine::conservativeUpperPlane(300.0,2800.0,2100.0);
 assert(flat["valid"].toBool());
 assert(flat["status"].toString()=="FLAT_SOURCE_HEIGHT_PLANE");
 assert(!flat["adjustedToTerrainPeak"].toBool());
 assert(qAbs(flat["sourceUpperPlaneMeters"].toDouble()-2400.0)<0.001);
 assert(qAbs(flat["upperPlaneMeters"].toDouble()-2400.0)<0.001);
 auto invalidTerrain=NotamGeometryEngine::conservativeUpperPlane(2500.0,2400.0,3000.0);
 assert(!invalidTerrain["valid"].toBool());
 // Compound NOTAM: one closed area plus two explicit circles must remain
 // three independent polygon components. Mixed vertical datums must fail closed.
 QVariantMap compound{{"originalText",
 "E)AIRSPACE CLSD AS FLW:\n"
 "1. AREA: 444741N0413513E-444145N0420555E-443855N0420723E-\n"
 "   443406N0422158E-444741N0413513E\n"
 "   650M AMSL-950M AMSL.\n"
 "2. WI CIRCLE RADIUS 1KM CENTRE 444310N0414605E\n"
 "   GND-950M AMSL.\n"
 "3. WI CIRCLE RADIUS 2KM CENTRE 443233N0422500E\n"
 "   GND-950M AMSL.\n"
 "F)GND G)950M AMSL"}};
 auto compoundResult=NotamGeometryEngine::normalize(compound);
 auto compoundGeometry=QJsonDocument::fromJson(compoundResult["geometryJson"].toString().toUtf8()).object();
 assert(compoundGeometry["type"].toString()=="MultiPolygon");
 auto components=compoundGeometry["coordinates"].toArray();
 assert(components.size()==3);
 assert(components.at(0).toArray().first().toArray().size()==5);
 assert(components.at(1).toArray().first().toArray().size()==73);
 assert(components.at(2).toArray().first().toArray().size()==73);
 assert(!compoundResult["geometryValidated"].toBool());
 assert(compoundResult["restrictionGeometryStatus"].toString()=="MISSING_OR_INCOMPATIBLE_VERTICAL_BOUNDS");


 // Full V7687/26 pattern: one AREA and five circles are repeated in a second
 // numbered AS FLW block. Keep six unique components, not twelve, and never
 // connect circle centres to the AREA boundary.
 QVariantMap v7687{{"originalText",
 "E)AIRSPACE CLSD AS FLW:\\n"
 "1. AREA: 444741N0413513E-444145N0420555E-443855N0420723E-\\n"
 "   443406N0422158E-443851N0422546E-443810N0422731E-\\n"
 "   442713N0422933E-443527N0420327E-443123N0415916E-\\n"
 "   442410N0415905E-442358N0415551E-443053N0415510E-\\n"
 "   443059N0415118E-443240N0415120E-443301N0415517E-\\n"
 "   443700N0415906E-443845N0415634E-443853N0415712E-\\n"
 "   443906N0415734E-443923N0415748E-443944N0415751E-\\n"
 "   444004N0415743E-444022N0415723E-444032N0415654E-\\n"
 "   444032N0415623E-444024N0415555E-444010N0415536E-\\n"
 "   443951N0415524E-443937N0415524E-444107N0415316E-\\n"
 "   444249N0414250E-443524N0414104E-443505N0413529E-\\n"
 "   444100N0413043E-444423N0413334E-444741N0413513E\\n"
 "   650M AMSL-950M AMSL.\\n"
 "2. WI CIRCLE RADIUS 1KM CENTRE 444310N0414605E\\n"
 "3. WI CIRCLE RADIUS 1KM CENTRE 443716N0420449E\\n"
 "4. WI CIRCLE RADIUS 2KM CENTRE 443233N0422500E\\n"
 "5. WI CIRCLE RADIUS 2KM CENTRE 443632N0420713E\\n"
 "6. WI CIRCLE RADIUS 2KM CENTRE 443528N0422419E\\n"
 "  F)GND G)950M AMSL\\n"
 "E)AIRSPACE CLSD AS FLW:\\n"
 "7. AREA: 444741N0413513E-444145N0420555E-443855N0420723E-\\n"
 "   443406N0422158E-443851N0422546E-443810N0422731E-\\n"
 "   442713N0422933E-443527N0420327E-443123N0415916E-\\n"
 "   442410N0415905E-442358N0415551E-443053N0415510E-\\n"
 "   443059N0415118E-443240N0415120E-443301N0415517E-\\n"
 "   443700N0415906E-443845N0415634E-443853N0415712E-\\n"
 "   443906N0415734E-443923N0415748E-443944N0415751E-\\n"
 "   444004N0415743E-444022N0415723E-444032N0415654E-\\n"
 "   444032N0415623E-444024N0415555E-444010N0415536E-\\n"
 "   443951N0415524E-443937N0415524E-444107N0415316E-\\n"
 "   444249N0414250E-443524N0414104E-443505N0413529E-\\n"
 "   444100N0413043E-444423N0413334E-444741N0413513E\\n"
 "   650M AMSL-950M AMSL.\\n"
 "8. WI CIRCLE RADIUS 1KM CENTRE 444310N0414605E\\n"
 "9. WI CIRCLE RADIUS 1KM CENTRE 443716N0420449E\\n"
 "10. WI CIRCLE RADIUS 2KM CENTRE 443233N0422500E\\n"
 "11. WI CIRCLE RADIUS 2KM CENTRE 443632N0420713E\\n"
 "12. WI CIRCLE RADIUS 2KM CENTRE 443528N0422419E\\n"
 "  F)GND G)950M AMSL"},
 {"lowerLimitRaw","GND"},{"upperLimitRaw","950M AMSL"}};
 auto v7687Result=NotamGeometryEngine::normalize(v7687);
 auto v7687Geo=QJsonDocument::fromJson(v7687Result["geometryJson"].toString().toUtf8()).object();
 assert(v7687Geo["type"].toString()=="MultiPolygon");
 auto v7687Parts=v7687Geo["coordinates"].toArray();
 assert(v7687Parts.size()==6);
 assert(v7687Parts.at(0).toArray().first().toArray().size()>30);
 for(int i=1;i<v7687Parts.size();++i)
   assert(v7687Parts.at(i).toArray().first().toArray().size()==73);

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