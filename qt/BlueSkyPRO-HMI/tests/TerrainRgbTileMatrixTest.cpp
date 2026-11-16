#include "TerrainRgbTileMatrix.h"

#include <QCoreApplication>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TerrainRgbTileCoordinate tile;
    assert(TerrainRgbTileMatrix::tileForCoordinate(0.0, 0.0, 0, &tile));
    assert(tile.x == 0 && tile.y == 0);
    assert(TerrainRgbTileMatrix::tileForCoordinate(0.0, 0.0, 1, &tile));
    assert(tile.x == 1 && tile.y == 1);
    assert(TerrainRgbTileMatrix::tileForCoordinate(90.0, 180.0, 14, &tile));
    assert(tile.x == 16383 && tile.y == 0);
    assert(TerrainRgbTileMatrix::tileForCoordinate(-90.0, -180.0, 14, &tile));
    assert(tile.x == 0 && tile.y == 16383);

    assert(!TerrainRgbTileMatrix::tileForCoordinate(91.0, 0.0, 2, &tile));
    assert(!TerrainRgbTileMatrix::tileForCoordinate(0.0, 0.0, 15, &tile));
    assert(!TerrainRgbTileMatrix::tileForCoordinate(0.0, 0.0, 2, nullptr));

    QVector<TerrainRgbTileCoordinate> tiles;
    assert(TerrainRgbTileMatrix::tilesForBounds(10.0, 20.0, 11.0, 21.0,
                                                 8, &tiles));
    assert(!tiles.isEmpty());
    for (const auto &item : tiles)
        assert(item.x >= 0 && item.x < 256 && item.y >= 0 && item.y < 256);

    // Antimeridian AOIs must split into the eastern and western edge columns.
    assert(TerrainRgbTileMatrix::tilesForBounds(-1.0, 179.0, 1.0, -179.0,
                                                 4, &tiles));
    bool hasEasternEdge = false;
    bool hasWesternEdge = false;
    for (const auto &item : tiles) {
        hasEasternEdge = hasEasternEdge || item.x == 15;
        hasWesternEdge = hasWesternEdge || item.x == 0;
    }
    assert(hasEasternEdge && hasWesternEdge);

    const auto previous = tiles;
    assert(!TerrainRgbTileMatrix::tilesForBounds(-80.0, -170.0, 80.0, 170.0,
                                                  14, &tiles, 10));
    assert(tiles == previous); // failed requests do not publish partial results
    assert(!TerrainRgbTileMatrix::tilesForBounds(1.0, 0.0, 1.0, 1.0,
                                                  4, &tiles));
    assert(!TerrainRgbTileMatrix::tilesForBounds(-1.0, 0.0, 1.0, 0.0,
                                                  4, &tiles));
    return 0;
}
