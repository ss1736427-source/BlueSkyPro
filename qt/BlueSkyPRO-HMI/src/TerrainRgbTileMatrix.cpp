#include "TerrainRgbTileMatrix.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMaxMercatorLatitude = 85.0511287798066;

bool validZoom(int zoom)
{
    return zoom >= 0 && zoom <= 14;
}

int tileX(double longitudeDeg, int count)
{
    const double normalized = std::clamp(longitudeDeg, -180.0, 180.0);
    const double raw = (normalized + 180.0) / 360.0 * count;
    return std::clamp(static_cast<int>(std::floor(raw)), 0, count - 1);
}

int tileY(double latitudeDeg, int count)
{
    const double latitude = std::clamp(latitudeDeg,
                                       -kMaxMercatorLatitude,
                                       kMaxMercatorLatitude);
    const double radians = latitude * kPi / 180.0;
    const double mercator = std::asinh(std::tan(radians));
    const double raw = (1.0 - mercator / kPi) * 0.5 * count;
    return std::clamp(static_cast<int>(std::floor(raw)), 0, count - 1);
}
}

bool TerrainRgbTileMatrix::tileForCoordinate(
    double latitudeDeg, double longitudeDeg, int zoom,
    TerrainRgbTileCoordinate *tile)
{
    if (!tile || !validZoom(zoom) || !std::isfinite(latitudeDeg)
        || !std::isfinite(longitudeDeg) || latitudeDeg < -90.0
        || latitudeDeg > 90.0 || longitudeDeg < -180.0
        || longitudeDeg > 180.0) {
        return false;
    }

    const int count = 1 << zoom;
    tile->x = tileX(longitudeDeg, count);
    tile->y = tileY(latitudeDeg, count);
    return true;
}

bool TerrainRgbTileMatrix::tilesForBounds(
    double southDeg, double westDeg, double northDeg, double eastDeg,
    int zoom, QVector<TerrainRgbTileCoordinate> *tiles, int maxTiles)
{
    if (!tiles || !validZoom(zoom) || maxTiles < 1
        || !std::isfinite(southDeg) || !std::isfinite(westDeg)
        || !std::isfinite(northDeg) || !std::isfinite(eastDeg)
        || southDeg < -90.0 || northDeg > 90.0 || southDeg >= northDeg
        || westDeg < -180.0 || westDeg > 180.0
        || eastDeg < -180.0 || eastDeg > 180.0 || westDeg == eastDeg) {
        return false;
    }

    const int count = 1 << zoom;
    const int minY = tileY(northDeg, count);
    const int maxY = tileY(southDeg, count);
    const int westX = tileX(westDeg, count);
    const int eastX = tileX(eastDeg, count);

    const int xRanges[2][2] = {
        {westX, westDeg <= eastDeg ? eastX : count - 1},
        {0, eastX}
    };
    const int rangeCount = westDeg <= eastDeg ? 1 : 2;

    qint64 required = 0;
    for (int range = 0; range < rangeCount; ++range) {
        required += static_cast<qint64>(maxY - minY + 1)
            * static_cast<qint64>(xRanges[range][1] - xRanges[range][0] + 1);
    }
    if (required <= 0 || required > maxTiles
        || required > std::numeric_limits<int>::max()) {
        return false;
    }

    QVector<TerrainRgbTileCoordinate> result;
    result.reserve(static_cast<int>(required));
    for (int y = minY; y <= maxY; ++y) {
        for (int range = 0; range < rangeCount; ++range) {
            for (int x = xRanges[range][0]; x <= xRanges[range][1]; ++x)
                result.append({x, y});
        }
    }

    *tiles = std::move(result);
    return true;
}
