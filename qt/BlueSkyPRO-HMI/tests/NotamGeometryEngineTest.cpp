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
 QVariantMap bad{{"text","Informational NOTAM only"}};auto d=NotamGeometryEngine::normalize(bad);assert(!d["geometryValidated"].toBool());return 0;
}