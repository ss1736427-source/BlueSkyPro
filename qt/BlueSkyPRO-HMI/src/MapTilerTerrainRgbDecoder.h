#pragma once

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QUrl>

class MapTilerTerrainRgbDecoder final
{
public:
    // MapTiler Terrain RGB v2 uses Mapbox RGB elevation encoding:
    // elevation_m = -10000 + (R * 65536 + G * 256 + B) * 0.1.
    static double decodeElevation(int red, int green, int blue);

    // Decode an individual RGB/WebP terrain tile. Only 256x256 and 512x512
    // tiles are accepted; unsupported image codecs fail closed.
    static bool decodeWebpTile(const QByteArray &payload, QImage *tile,
                               QString *error = nullptr);

    // Bilinear sample in tile pixel coordinates. Transparent/NoData pixels fail closed.
    static bool sampleElevation(const QImage &tile, double pixelX, double pixelY,
                                double *elevationM, QString *error = nullptr);

    // Canonical MapTiler Terrain RGB v2 tile URL. Do not log this URL: it contains the API key.
    static QUrl buildTileUrl(int zoom, int x, int y, const QString &apiKey,
                             QString *error = nullptr);
};
