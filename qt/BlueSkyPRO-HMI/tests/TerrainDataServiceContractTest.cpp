#include "TerrainDataService.h"
#include "TerrainRgbRouteSampler.h"
#include <QCoreApplication>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <QHash>
#include <QImage>
#include <QRgb>

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

    // Exercise bilinear interpolation across the x=256 tile seam at z=1.
    // The route midpoint maps to world pixel x=255.5; adjacent tiles contain
    // deliberately different elevations to expose seam discontinuity.
    const auto encodedTerrainPixel = [](int elevationM) {
        const int encoded = qRound((elevationM + 10000.0) * 10.0);
        return qRgba((encoded >> 16) & 0xff, (encoded >> 8) & 0xff,
                     encoded & 0xff, 255);
    };
    QImage westTile(256, 256, QImage::Format_ARGB32);
    QImage eastTile(256, 256, QImage::Format_ARGB32);
    westTile.fill(encodedTerrainPixel(100));
    eastTile.fill(encodedTerrainPixel(200));
    QHash<QString, QImage> tiles{
        {QStringLiteral("1/0/1"), westTile},
        {QStringLiteral("1/1/1"), eastTile}
    };
    const QVariantMap routeAcrossSeam{
        {QStringLiteral("coordinateReferenceSystem"), QStringLiteral("EPSG:4326")},
        {QStringLiteral("waypoints"), QVariantList{
            QVariantMap{{QStringLiteral("latitudeDeg"), 0.0},
                        {QStringLiteral("longitudeDeg"), -0.35246182}},
            QVariantMap{{QStringLiteral("latitudeDeg"), 0.0},
                        {QStringLiteral("longitudeDeg"), -0.35066318}}
        }}
    };
    assert(TerrainRgbRouteSampler::sampleRouteFromDecodedTiles(
        routeAcrossSeam, 1, tiles, &samples, &error));
    assert(samples.size() >= 2);
    double closestProgress = 1.0;
    double midpointElevation = 0.0;
    for (const QVariant &sample : samples) {
        const QVariantMap point = sample.toMap();
        const double distance = std::abs(point.value(QStringLiteral("progress")).toDouble() - 0.5);
        if (distance < closestProgress) {
            closestProgress = distance;
            midpointElevation = point.value(QStringLiteral("elevationM")).toDouble();
        }
        assert(point.value(QStringLiteral("evidenceClass")).toString()
               == QStringLiteral("VISUAL_ONLY"));
    }
    assert(closestProgress < 1e-6);
    assert(std::abs(midpointElevation - 150.0) < 0.2);
    const QString visualSnapshotId = samples.first().toMap()
        .value(QStringLiteral("snapshotId")).toString();
    assert(visualSnapshotId.startsWith(QStringLiteral("MAPTILER-VISUAL-SHA256:")));
    for (const QVariant &sample : samples) {
        const QVariantMap point = sample.toMap();
        assert(point.value(QStringLiteral("snapshotId")).toString() == visualSnapshotId);
        assert(point.value(QStringLiteral("sourceId")).toString()
               == QStringLiteral("MAPTILER_TERRAIN_RGB_V2"));
        assert(point.value(QStringLiteral("verticalDatum")).toString()
               == QStringLiteral("UNKNOWN"));
        assert(point.value(QStringLiteral("verticalAccuracy")).toString()
               == QStringLiteral("UNKNOWN"));
        assert(point.value(QStringLiteral("evidenceClass")).toString()
               == QStringLiteral("VISUAL_ONLY"));
    }

    // A changed source tile must produce a different immutable snapshot ID.
    eastTile.fill(encodedTerrainPixel(210));
    tiles.insert(QStringLiteral("1/1/1"), eastTile);
    assert(TerrainRgbRouteSampler::sampleRouteFromDecodedTiles(
        routeAcrossSeam, 1, tiles, &samples, &error));
    assert(samples.first().toMap().value(QStringLiteral("snapshotId")).toString()
           != visualSnapshotId);

    // Transparent Terrain RGB pixels are NoData: reject the whole profile,
    // never return a partial series.
    eastTile.setPixel(0, 0, qRgba(0, 0, 0, 0));
    tiles.insert(QStringLiteral("1/1/1"), eastTile);
    assert(!TerrainRgbRouteSampler::sampleRouteFromDecodedTiles(
        routeAcrossSeam, 1, tiles, &samples, &error));
    assert(samples.isEmpty());
    assert(error == QStringLiteral("ROUTE_POINT_OUTSIDE_TERRAIN_TILE_COVERAGE_OR_NODATA"));
    return 0;
}
