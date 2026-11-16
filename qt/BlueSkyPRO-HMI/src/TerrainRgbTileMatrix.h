#pragma once

#include <QVector>

struct TerrainRgbTileCoordinate {
    int x{0};
    int y{0};

    bool operator==(const TerrainRgbTileCoordinate &other) const
    {
        return x == other.x && y == other.y;
    }
};

class TerrainRgbTileMatrix final
{
public:
    // Converts WGS84 coordinates to XYZ tile coordinates. Longitude is
    // normalized at the antimeridian; latitude is clamped to Web Mercator.
    static bool tileForCoordinate(double latitudeDeg, double longitudeDeg,
                                  int zoom, TerrainRgbTileCoordinate *tile);

    // Returns every XYZ tile intersecting an AOI. west > east denotes an AOI
    // crossing the antimeridian. The operation fails before allocation when
    // the requested tile count exceeds maxTiles.
    static bool tilesForBounds(double southDeg, double westDeg,
                               double northDeg, double eastDeg,
                               int zoom, QVector<TerrainRgbTileCoordinate> *tiles,
                               int maxTiles = 4096);
};
