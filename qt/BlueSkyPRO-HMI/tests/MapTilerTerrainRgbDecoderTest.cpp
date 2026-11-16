#include "MapTilerTerrainRgbDecoder.h"

#include <QCoreApplication>
#include <QUrlQuery>

#include <cassert>
#include <cmath>

static QRgb encode(double elevationM, int alpha = 255)
{
    const auto encoded = static_cast<quint32>(std::llround((elevationM + 10000.0) * 10.0));
    return qRgba((encoded >> 16) & 0xff, (encoded >> 8) & 0xff, encoded & 0xff, alpha);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const double known = MapTilerTerrainRgbDecoder::decodeElevation(1, 134, 160);
    const double expected = -10000.0 + (1.0 * 65536.0 + 134.0 * 256.0 + 160.0) * 0.1;
    assert(std::abs(known - expected) < 1e-9);
    assert(!std::isfinite(MapTilerTerrainRgbDecoder::decodeElevation(-1, 0, 0)));
    assert(!std::isfinite(MapTilerTerrainRgbDecoder::decodeElevation(0, 256, 0)));

    QImage tile(2, 2, QImage::Format_ARGB32);
    tile.setPixel(0, 0, encode(100.0));
    tile.setPixel(1, 0, encode(200.0));
    tile.setPixel(0, 1, encode(300.0));
    tile.setPixel(1, 1, encode(400.0));

    double sampled = 0.0;
    QString error;
    assert(MapTilerTerrainRgbDecoder::sampleElevation(tile, 0.5, 0.5, &sampled, &error));
    assert(std::abs(sampled - 250.0) <= 0.051);
    assert(error.isEmpty());

    assert(!MapTilerTerrainRgbDecoder::sampleElevation(tile, -0.1, 0.5, &sampled, &error));
    assert(!error.isEmpty());
    assert(!MapTilerTerrainRgbDecoder::sampleElevation(tile, 0.5, 0.5, nullptr, &error));

    tile.setPixel(1, 1, encode(400.0, 0));
    assert(!MapTilerTerrainRgbDecoder::sampleElevation(tile, 0.5, 0.5, &sampled, &error));

    const QUrl url = MapTilerTerrainRgbDecoder::buildTileUrl(12, 2200, 1400,
                                                             QStringLiteral("test-key"), &error);
    assert(url.isValid());
    assert(url.host() == QStringLiteral("api.maptiler.com"));
    assert(url.path() == QStringLiteral("/tiles/terrain-rgb-v2/12/2200/1400.webp"));
    assert(QUrlQuery(url).queryItemValue(QStringLiteral("key")) == QStringLiteral("test-key"));
    assert(error.isEmpty());

    assert(!MapTilerTerrainRgbDecoder::buildTileUrl(15, 0, 0, QStringLiteral("key"), &error).isValid());
    assert(!MapTilerTerrainRgbDecoder::buildTileUrl(2, 4, 0, QStringLiteral("key"), &error).isValid());
    assert(!MapTilerTerrainRgbDecoder::buildTileUrl(2, 0, 0, QString(), &error).isValid());

    return 0;
}
