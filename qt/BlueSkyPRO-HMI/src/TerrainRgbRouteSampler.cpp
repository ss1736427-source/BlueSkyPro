#include "TerrainRgbRouteSampler.h"

#include "MapTilerTerrainRgbDecoder.h"

#include <QFile>
#include <QVariant>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <limits>
#include <QVector>

namespace
{
constexpr double kEarthRadiusM = 6371008.8;
constexpr double kMercatorLatitudeLimit = 85.0511287798066;
constexpr double kPi = 3.14159265358979323846;

void setError(QString *target, const QString &value)
{
    if (target)
        *target = value;
}

struct Coordinate {
    double latitude{0.0};
    double longitude{0.0};
};

double radians(double degrees) { return degrees * kPi / 180.0; }

double haversineDistance(const Coordinate &a, const Coordinate &b)
{
    const double lat1 = radians(a.latitude);
    const double lat2 = radians(b.latitude);
    const double dLat = lat2 - lat1;
    const double dLon = radians(b.longitude - a.longitude);
    const double h = std::sin(dLat / 2.0) * std::sin(dLat / 2.0)
        + std::cos(lat1) * std::cos(lat2)
        * std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    return 2.0 * kEarthRadiusM * std::asin(std::sqrt(std::clamp(h, 0.0, 1.0)));
}

Coordinate interpolateGreatCircle(const Coordinate &a, const Coordinate &b, double t)
{
    const double lat1 = radians(a.latitude);
    const double lon1 = radians(a.longitude);
    const double lat2 = radians(b.latitude);
    const double lon2 = radians(b.longitude);
    const double ax = std::cos(lat1) * std::cos(lon1);
    const double ay = std::cos(lat1) * std::sin(lon1);
    const double az = std::sin(lat1);
    const double bx = std::cos(lat2) * std::cos(lon2);
    const double by = std::cos(lat2) * std::sin(lon2);
    const double bz = std::sin(lat2);
    const double dot = std::clamp(ax * bx + ay * by + az * bz, -1.0, 1.0);
    const double omega = std::acos(dot);
    if (omega < 1e-12)
        return {a.latitude + (b.latitude - a.latitude) * t,
                a.longitude + (b.longitude - a.longitude) * t};
    const double sinOmega = std::sin(omega);
    if (std::abs(sinOmega) < 1e-12)
        return {a.latitude + (b.latitude - a.latitude) * t,
                a.longitude + (b.longitude - a.longitude) * t};
    const double wa = std::sin((1.0 - t) * omega) / sinOmega;
    const double wb = std::sin(t * omega) / sinOmega;
    const double x = wa * ax + wb * bx;
    const double y = wa * ay + wb * by;
    const double z = wa * az + wb * bz;
    return {std::atan2(z, std::hypot(x, y)) * 180.0 / kPi,
            std::atan2(y, x) * 180.0 / kPi};
}

QString tileKey(int zoom, int x, int y)
{
    return QStringLiteral("%1/%2/%3").arg(zoom).arg(x).arg(y);
}

bool sampleWorldPixel(int zoom, int tileSize, const QHash<QString, QImage> &tiles,
                      qint64 worldPixelX, qint64 worldPixelY, double *elevation)
{
    const qint64 worldSize = static_cast<qint64>(1 << zoom) * tileSize;
    if (worldSize <= 0 || !elevation)
        return false;
    worldPixelX %= worldSize;
    if (worldPixelX < 0)
        worldPixelX += worldSize;
    worldPixelY = std::clamp<qint64>(worldPixelY, 0, worldSize - 1);
    const int tileX = static_cast<int>(worldPixelX / tileSize);
    const int tileY = static_cast<int>(worldPixelY / tileSize);
    const int pixelX = static_cast<int>(worldPixelX % tileSize);
    const int pixelY = static_cast<int>(worldPixelY % tileSize);
    const auto it = tiles.constFind(tileKey(zoom, tileX, tileY));
    if (it == tiles.cend() || it->isNull())
        return false;
    const QRgb pixel = it->pixel(pixelX, pixelY);
    if (qAlpha(pixel) == 0)
        return false;
    *elevation = MapTilerTerrainRgbDecoder::decodeElevation(
        qRed(pixel), qGreen(pixel), qBlue(pixel));
    return std::isfinite(*elevation);
}

bool sampleCoordinate(const Coordinate &coordinate, int zoom, int tileSize,
                      const QHash<QString, QImage> &tiles, double *elevation)
{
    const qint64 worldSize = static_cast<qint64>(1 << zoom) * tileSize;
    const double latitude = std::clamp(coordinate.latitude,
                                       -kMercatorLatitudeLimit,
                                       kMercatorLatitudeLimit);
    double longitude = std::fmod(coordinate.longitude + 180.0, 360.0);
    if (longitude < 0.0)
        longitude += 360.0;
    longitude -= 180.0;
    const bool positiveDateLine = coordinate.longitude >= 180.0 - 1e-12;
    const double worldX = positiveDateLine ? static_cast<double>(worldSize - 1)
        : (longitude + 180.0) / 360.0 * static_cast<double>(worldSize);
    const double latRad = radians(latitude);
    const double worldY = (1.0 - std::asinh(std::tan(latRad)) / kPi) / 2.0
        * static_cast<double>(worldSize);
    const qint64 x0 = static_cast<qint64>(std::floor(worldX));
    const qint64 y0 = std::clamp<qint64>(
        static_cast<qint64>(std::floor(worldY)), 0, worldSize - 1);
    const qint64 x1 = (x0 + 1) % worldSize;
    const qint64 y1 = std::min<qint64>(y0 + 1, worldSize - 1);
    const double tx = worldX - std::floor(worldX);
    const double ty = std::clamp(worldY - std::floor(worldY), 0.0, 1.0);

    double values[4]{};
    if (!sampleWorldPixel(zoom, tileSize, tiles, x0, y0, &values[0])
        || !sampleWorldPixel(zoom, tileSize, tiles, x1, y0, &values[1])
        || !sampleWorldPixel(zoom, tileSize, tiles, x0, y1, &values[2])
        || !sampleWorldPixel(zoom, tileSize, tiles, x1, y1, &values[3]))
        return false;
    *elevation = (1.0 - ty) * ((1.0 - tx) * values[0] + tx * values[1])
        + ty * ((1.0 - tx) * values[2] + tx * values[3]);
    return std::isfinite(*elevation);
}
}

bool TerrainRgbRouteSampler::sampleRoute(const QVariantMap &routeSnapshot,
                                         const QVariantList &tileManifest,
                                         QVariantList *samples,
                                         QString *error,
                                         double maximumSpacingM,
                                         int maximumSamples)
{
    if (!samples || !std::isfinite(maximumSpacingM) || maximumSpacingM <= 0.0
        || maximumSamples < 2) {
        setError(error, QStringLiteral("INVALID_SAMPLER_ARGUMENTS"));
        return false;
    }
    samples->clear();
    if (routeSnapshot.value(QStringLiteral("coordinateReferenceSystem")).toString()
            != QStringLiteral("EPSG:4326")) {
        setError(error, QStringLiteral("ROUTE_CRS_MUST_BE_EPSG_4326"));
        return false;
    }
    const int zoom = tileManifest.isEmpty() ? -1
        : tileManifest.first().toMap().value(QStringLiteral("zoom")).toInt();
    if (zoom < 0 || zoom > 14) {
        setError(error, QStringLiteral("TERRAIN_TILE_MANIFEST_INVALID"));
        return false;
    }

    QHash<QString, QImage> tiles;
    int tileSize = 0;
    for (const QVariant &entry : tileManifest) {
        const QVariantMap metadata = entry.toMap();
        if (metadata.value(QStringLiteral("evidenceClass")).toString()
                != QStringLiteral("VISUAL_ONLY")
            || metadata.value(QStringLiteral("zoom")).toInt() != zoom) {
            setError(error, QStringLiteral("TERRAIN_TILE_PROVENANCE_OR_ZOOM_MISMATCH"));
            return false;
        }
        const int x = metadata.value(QStringLiteral("x"), -1).toInt();
        const int y = metadata.value(QStringLiteral("y"), -1).toInt();
        const QString path = metadata.value(QStringLiteral("cachePath")).toString();
        QFile file(path);
        if (x < 0 || x >= (1 << zoom) || y < 0 || y >= (1 << zoom)
            || path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
            setError(error, QStringLiteral("TERRAIN_TILE_CACHE_ENTRY_UNAVAILABLE"));
            return false;
        }
        QImage image;
        if (!MapTilerTerrainRgbDecoder::decodeWebpTile(file.readAll(), &image, nullptr)) {
            setError(error, QStringLiteral("TERRAIN_TILE_CACHE_DECODE_FAILED"));
            return false;
        }
        if (tileSize == 0)
            tileSize = image.width();
        if (image.width() != tileSize || image.height() != tileSize) {
            setError(error, QStringLiteral("MIXED_TERRAIN_TILE_DIMENSIONS"));
            return false;
        }
        tiles.insert(tileKey(zoom, x, y), image);
    }
    if (tiles.isEmpty() || tileSize < 2) {
        setError(error, QStringLiteral("TERRAIN_TILE_SET_EMPTY"));
        return false;
    }
    return sampleRouteFromDecodedTiles(routeSnapshot, zoom, tiles, samples,
                                        error, maximumSpacingM, maximumSamples);
}

bool TerrainRgbRouteSampler::sampleRouteFromDecodedTiles(
    const QVariantMap &routeSnapshot, int zoom,
    const QHash<QString, QImage> &tiles, QVariantList *samples,
    QString *error, double maximumSpacingM, int maximumSamples)
{
    if (!samples || !std::isfinite(maximumSpacingM) || maximumSpacingM <= 0.0
        || maximumSamples < 2 || zoom < 0 || zoom > 14) {
        setError(error, QStringLiteral("INVALID_SAMPLER_ARGUMENTS"));
        return false;
    }
    samples->clear();
    if (routeSnapshot.value(QStringLiteral("coordinateReferenceSystem")).toString()
            != QStringLiteral("EPSG:4326")) {
        setError(error, QStringLiteral("ROUTE_CRS_MUST_BE_EPSG_4326"));
        return false;
    }
    int tileSize = 0;
    for (auto it = tiles.cbegin(); it != tiles.cend(); ++it) {
        if (it.value().isNull() || it.value().width() != it.value().height()
            || it.value().width() < 2
            || (tileSize != 0 && it.value().width() != tileSize)) {
            setError(error, QStringLiteral("INVALID_DECODED_TILE_SET"));
            return false;
        }
        tileSize = it.value().width();
    }
    if (tiles.isEmpty()) {
        setError(error, QStringLiteral("TERRAIN_TILE_SET_EMPTY"));
        return false;
    }

    const QVariantList waypoints = routeSnapshot.value(QStringLiteral("waypoints")).toList();
    if (waypoints.size() < 2) {
        setError(error, QStringLiteral("ROUTE_REQUIRES_AT_LEAST_TWO_WAYPOINTS"));
        return false;
    }
    QVector<Coordinate> coordinates;
    coordinates.reserve(waypoints.size());
    for (const QVariant &value : waypoints) {
        const QVariantMap point = value.toMap();
        const double lat = point.value(QStringLiteral("latitudeDeg"),
                                       std::numeric_limits<double>::quiet_NaN()).toDouble();
        const double lon = point.value(QStringLiteral("longitudeDeg"),
                                       std::numeric_limits<double>::quiet_NaN()).toDouble();
        if (!std::isfinite(lat) || !std::isfinite(lon)
            || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) {
            setError(error, QStringLiteral("ROUTE_WAYPOINT_COORDINATE_INVALID"));
            return false;
        }
        const Coordinate candidate{lat, lon};
        if (coordinates.isEmpty()
            || haversineDistance(coordinates.last(), candidate) > 0.01)
            coordinates.push_back(candidate);
    }

    if (coordinates.size() < 2) {
        setError(error, QStringLiteral("ROUTE_HAS_NO_DISTINCT_WAYPOINTS"));
        return false;
    }

    QVector<double> segmentDistances;
    double totalDistance = 0.0;
    for (int i = 1; i < coordinates.size(); ++i) {
        const double distance = haversineDistance(coordinates[i - 1], coordinates[i]);
        if (!std::isfinite(distance)) {
            setError(error, QStringLiteral("ROUTE_GEODESIC_DISTANCE_INVALID"));
            return false;
        }
        segmentDistances.push_back(distance);
        totalDistance += distance;
    }
    if (totalDistance <= 0.01) {
        setError(error, QStringLiteral("ROUTE_LENGTH_TOO_SHORT"));
        return false;
    }
    const int estimatedSamples = 1 + static_cast<int>(std::ceil(totalDistance / maximumSpacingM))
        + coordinates.size();
    if (estimatedSamples > maximumSamples) {
        setError(error, QStringLiteral("ROUTE_SAMPLE_LIMIT_EXCEEDED"));
        return false;
    }

    double cumulativeDistance = 0.0;
    for (int segment = 0; segment < segmentDistances.size(); ++segment) {
        const double distance = segmentDistances[segment];
        const int steps = std::max(1, static_cast<int>(std::ceil(distance / maximumSpacingM)));
        const int firstStep = segment == 0 ? 0 : 1;
        for (int step = firstStep; step <= steps; ++step) {
            const double fraction = static_cast<double>(step) / steps;
            const Coordinate coordinate = interpolateGreatCircle(
                coordinates[segment], coordinates[segment + 1], fraction);
            double elevation = 0.0;
            if (!sampleCoordinate(coordinate, zoom, tileSize, tiles, &elevation)) {
                samples->clear();
                setError(error, QStringLiteral("ROUTE_POINT_OUTSIDE_TERRAIN_TILE_COVERAGE_OR_NODATA"));
                return false;
            }
            const double progress = (cumulativeDistance + distance * fraction) / totalDistance;
            samples->push_back(QVariantMap{
                {QStringLiteral("progress"), progress},
                {QStringLiteral("elevationM"), elevation},
                {QStringLiteral("evidenceClass"), QStringLiteral("VISUAL_ONLY")}
            });
            if (samples->size() > maximumSamples) {
                samples->clear();
                setError(error, QStringLiteral("ROUTE_SAMPLE_LIMIT_EXCEEDED"));
                return false;
            }
        }
        cumulativeDistance += distance;
    }
    if (samples->size() < 2) {
        samples->clear();
        setError(error, QStringLiteral("ROUTE_PROFILE_INCOMPLETE"));
        return false;
    }
    if (error)
        error->clear();
    return true;
}
