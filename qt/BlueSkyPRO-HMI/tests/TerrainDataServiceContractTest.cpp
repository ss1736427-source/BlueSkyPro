#include "TerrainDataService.h"
#include "TerrainRgbRouteSampler.h"
#include <QCoreApplication>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TerrainDataService service;
    assert(service.state() == QStringLiteral("IDLE"));
    assert(service.completedTiles() == 0);
    assert(service.totalTiles() == 0);
    assert(!service.requestVisualContextTiles(QString(), 10.0, 20.0, 11.0, 21.0,
                                               8, QStringLiteral("key")));
    assert(!service.requestVisualContextTiles(QStringLiteral("mission"), 10.0, 20.0,
                                               11.0, 21.0, 8, QString()));
    assert(!service.requestVisualContextTiles(QStringLiteral("mission"), 10.0, 20.0,
                                               11.0, 21.0, 15, QStringLiteral("key")));
    assert(service.state() == QStringLiteral("IDLE"));

    QVariantList samples;
    QString error;
    QVariantMap route{
        {QStringLiteral("coordinateReferenceSystem"), QStringLiteral("EPSG:3857")}
    };
    assert(!TerrainRgbRouteSampler::sampleRoute(route, {}, &samples, &error));
    assert(samples.isEmpty());
    assert(error == QStringLiteral("ROUTE_CRS_MUST_BE_EPSG_4326"));
    route.insert(QStringLiteral("coordinateReferenceSystem"), QStringLiteral("EPSG:4326"));
    assert(!TerrainRgbRouteSampler::sampleRoute(route, {}, &samples, &error));
    assert(error == QStringLiteral("TERRAIN_TILE_MANIFEST_INVALID"));
    return 0;
}
