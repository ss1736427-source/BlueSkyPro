#include "MapTilerTerrainRgbDecoder.h"

#include <QUrlQuery>

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
void setError(QString *target, const QString &message)
{
    if (target)
        *target = message;
}

bool validChannel(int value)
{
    return value >= 0 && value <= 255;
}

bool pixelElevation(const QImage &image, int x, int y, double *value)
{
    const QRgb pixel = image.pixel(x, y);
    if (qAlpha(pixel) != 255)
        return false;
    const double elevation = MapTilerTerrainRgbDecoder::decodeElevation(
        qRed(pixel), qGreen(pixel), qBlue(pixel));
    if (!std::isfinite(elevation))
        return false;
    *value = elevation;
    return true;
}
}

double MapTilerTerrainRgbDecoder::decodeElevation(int red, int green, int blue)
{
    if (!validChannel(red) || !validChannel(green) || !validChannel(blue))
        return std::numeric_limits<double>::quiet_NaN();
    const quint32 encoded = (static_cast<quint32>(red) << 16)
        | (static_cast<quint32>(green) << 8)
        | static_cast<quint32>(blue);
    return -10000.0 + static_cast<double>(encoded) * 0.1;
}

bool MapTilerTerrainRgbDecoder::decodeWebpTile(const QByteArray &payload,
                                                QImage *tile, QString *error)
{
    if (!tile) {
        setError(error, QStringLiteral("tile output pointer is null"));
        return false;
    }
    if (payload.isEmpty()) {
        setError(error, QStringLiteral("Terrain RGB tile payload is empty"));
        return false;
    }
    const QImage decoded = QImage::fromData(payload, "WEBP");
    if (decoded.isNull()) {
        setError(error, QStringLiteral("WebP decoder unavailable or tile payload invalid"));
        return false;
    }
    if ((decoded.width() != 256 && decoded.width() != 512)
        || decoded.width() != decoded.height()) {
        setError(error, QStringLiteral("Terrain RGB tile must be square 256x256 or 512x512"));
        return false;
    }
    *tile = decoded.convertToFormat(QImage::Format_ARGB32);
    if (tile->isNull()) {
        setError(error, QStringLiteral("Terrain RGB tile pixel conversion failed"));
        return false;
    }
    if (error)
        error->clear();
    return true;
}

bool MapTilerTerrainRgbDecoder::sampleElevation(const QImage &tile,
                                                 double pixelX, double pixelY,
                                                 double *elevationM,
                                                 QString *error)
{
    if (!elevationM || tile.isNull() || tile.width() < 2 || tile.height() < 2
        || !std::isfinite(pixelX) || !std::isfinite(pixelY)
        || pixelX < 0.0 || pixelY < 0.0
        || pixelX > static_cast<double>(tile.width() - 1)
        || pixelY > static_cast<double>(tile.height() - 1)) {
        setError(error, QStringLiteral("invalid tile or sample coordinates"));
        return false;
    }

    const int x0 = static_cast<int>(std::floor(pixelX));
    const int y0 = static_cast<int>(std::floor(pixelY));
    const int x1 = std::min(x0 + 1, tile.width() - 1);
    const int y1 = std::min(y0 + 1, tile.height() - 1);
    const double tx = pixelX - x0;
    const double ty = pixelY - y0;
    const double weights[4] = {
        (1.0 - tx) * (1.0 - ty), tx * (1.0 - ty),
        (1.0 - tx) * ty, tx * ty
    };
    const int xs[4] = {x0, x1, x0, x1};
    const int ys[4] = {y0, y0, y1, y1};

    double result = 0.0;
    for (int i = 0; i < 4; ++i) {
        if (weights[i] <= 1e-12)
            continue;
        double sample = 0.0;
        if (!pixelElevation(tile, xs[i], ys[i], &sample)) {
            setError(error, QStringLiteral("Terrain RGB sample intersects a transparent/NoData pixel"));
            return false;
        }
        result += weights[i] * sample;
    }
    if (!std::isfinite(result)) {
        setError(error, QStringLiteral("decoded terrain elevation is non-finite"));
        return false;
    }
    *elevationM = result;
    if (error)
        error->clear();
    return true;
}

QUrl MapTilerTerrainRgbDecoder::buildTileUrl(int zoom, int x, int y,
                                              const QString &apiKey,
                                              QString *error)
{
    if (zoom < 0 || zoom > 14 || apiKey.trimmed().isEmpty()) {
        setError(error, QStringLiteral("zoom must be 0..14 and API key must be configured"));
        return {};
    }
    const int tileCount = 1 << zoom;
    if (x < 0 || y < 0 || x >= tileCount || y >= tileCount) {
        setError(error, QStringLiteral("XYZ tile coordinate is outside the zoom matrix"));
        return {};
    }
    QUrl url(QStringLiteral("https://api.maptiler.com/tiles/terrain-rgb-v2/%1/%2/%3.webp")
                 .arg(zoom).arg(x).arg(y));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("key"), apiKey);
    url.setQuery(query);
    if (error)
        error->clear();
    return url;
}
