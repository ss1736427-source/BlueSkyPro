#include "GeoTiffTerrainReader.h"

#include <gdal.h>
#include <cpl_conv.h>
#include <ogr_srs_api.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <vector>
#include <utility>

namespace
{
constexpr int kMaximumRasterDimension = 8192;
constexpr std::size_t kMaximumRasterCells = 4000000;

void setError(QString *target, const QString &message)
{
    if (target)
        *target = message;
}

void initializeGdal()
{
    static std::once_flag once;
    std::call_once(once, []() {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString gdalData = QDir(appDir).filePath(QStringLiteral("gdal-data"));
        const QString projData = QDir(appDir).filePath(QStringLiteral("proj-data"));
        if (QDir(gdalData).exists())
            CPLSetConfigOption("GDAL_DATA", QDir::toNativeSeparators(gdalData).toUtf8().constData());
        if (QDir(projData).exists()) {
            const QByteArray path = QDir::toNativeSeparators(projData).toUtf8();
            CPLSetConfigOption("PROJ_DATA", path.constData());
            CPLSetConfigOption("PROJ_LIB", path.constData());
        }
        GDALAllRegister();
    });
}

QString sha256File(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(error, QStringLiteral("GEOTIFF_FILE_OPEN_FAILED"));
        return {};
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        setError(error, QStringLiteral("GEOTIFF_CHECKSUM_READ_FAILED"));
        return {};
    }
    return QString::fromLatin1(hash.result().toHex());
}

bool isMetreUnit(const QString &unit)
{
    const QString normalized = unit.trimmed().toLower();
    return normalized == QStringLiteral("m")
        || normalized == QStringLiteral("meter")
        || normalized == QStringLiteral("meters")
        || normalized == QStringLiteral("metre")
        || normalized == QStringLiteral("metres");
}

bool isWgs84Geographic(const char *projection)
{
    if (!projection || !*projection)
        return false;
    OGRSpatialReferenceH actual = OSRNewSpatialReference(nullptr);
    OGRSpatialReferenceH expected = OSRNewSpatialReference(nullptr);
    if (!actual || !expected) {
        if (actual) OSRDestroySpatialReference(actual);
        if (expected) OSRDestroySpatialReference(expected);
        return false;
    }
    const bool parsed = OSRSetFromUserInput(actual, projection) == OGRERR_NONE
        && OSRSetFromUserInput(expected, "EPSG:4326") == OGRERR_NONE;
    const bool same = parsed && OSRIsSame(actual, expected) != 0;
    OSRDestroySpatialReference(actual);
    OSRDestroySpatialReference(expected);
    return same;
}
}

namespace bluesky::planning {

bool GeoTiffTerrainReader::readForInspection(const QString &filePath,
                                             TerrainDemGrid *grid,
                                             QString *error)
{
    if (!grid) {
        setError(error, QStringLiteral("GEOTIFF_OUTPUT_GRID_REQUIRED"));
        return false;
    }
    *grid = TerrainDemGrid{};
    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile() || info.size() <= 0) {
        setError(error, QStringLiteral("GEOTIFF_FILE_NOT_FOUND_OR_EMPTY"));
        return false;
    }

    initializeGdal();
    const QByteArray nativePath = QFile::encodeName(info.absoluteFilePath());
    const char *allowedDrivers[] = {"GTiff", nullptr};
    GDALDatasetH dataset = GDALOpenEx(nativePath.constData(),
                                      GDAL_OF_RASTER | GDAL_OF_READONLY,
                                      allowedDrivers, nullptr, nullptr);
    if (!dataset) {
        setError(error, QStringLiteral("GEOTIFF_OPEN_FAILED: ")
                        + QString::fromUtf8(CPLGetLastErrorMsg()));
        return false;
    }

    const auto closeDataset = [&dataset]() {
        if (dataset) {
            GDALClose(dataset);
            dataset = nullptr;
        }
    };
    bool ok = false;
    do {
        if (QString::fromLatin1(GDALGetDriverShortName(GDALGetDatasetDriver(dataset)))
            != QStringLiteral("GTiff")) {
            setError(error, QStringLiteral("GEOTIFF_DRIVER_REQUIRED"));
            break;
        }

        const int width = GDALGetRasterXSize(dataset);
        const int height = GDALGetRasterYSize(dataset);
        const int bandCount = GDALGetRasterCount(dataset);
        if (width < 2 || height < 2 || width > kMaximumRasterDimension
            || height > kMaximumRasterDimension || bandCount != 1
            || static_cast<std::size_t>(width) * static_cast<std::size_t>(height)
                   > kMaximumRasterCells) {
            setError(error, QStringLiteral("GEOTIFF_DIMENSIONS_BAND_COUNT_OR_CELL_LIMIT_INVALID"));
            break;
        }

        if (!isWgs84Geographic(GDALGetProjectionRef(dataset))) {
            setError(error, QStringLiteral("GEOTIFF_HORIZONTAL_CRS_MUST_BE_EPSG_4326"));
            break;
        }

        double transform[6]{};
        if (GDALGetGeoTransform(dataset, transform) != CE_None
            || !std::isfinite(transform[0]) || !std::isfinite(transform[1])
            || !std::isfinite(transform[3]) || !std::isfinite(transform[5])
            || !std::isfinite(transform[2]) || !std::isfinite(transform[4])
            || transform[1] <= 0.0 || transform[5] >= 0.0
            || std::abs(transform[2]) > 1e-12 || std::abs(transform[4]) > 1e-12) {
            setError(error, QStringLiteral("GEOTIFF_MUST_BE_REGULAR_NORTH_UP_UNROTATED"));
            break;
        }

        GDALRasterBandH band = GDALGetRasterBand(dataset, 1);
        if (!band) {
            setError(error, QStringLiteral("GEOTIFF_ELEVATION_BAND_MISSING"));
            break;
        }
        const QString unit = QString::fromUtf8(GDALGetRasterUnitType(band));
        if (!isMetreUnit(unit)) {
            setError(error, QStringLiteral("GEOTIFF_ELEVATION_UNIT_MUST_BE_METRES"));
            break;
        }

        int hasScale = FALSE;
        int hasOffset = FALSE;
        const double scale = GDALGetRasterScale(band, &hasScale);
        const double offset = GDALGetRasterOffset(band, &hasOffset);
        const double appliedScale = hasScale ? scale : 1.0;
        const double appliedOffset = hasOffset ? offset : 0.0;
        if (!std::isfinite(appliedScale) || appliedScale == 0.0
            || !std::isfinite(appliedOffset)) {
            setError(error, QStringLiteral("GEOTIFF_SCALE_OR_OFFSET_INVALID"));
            break;
        }

        int hasNoData = FALSE;
        const double noData = GDALGetRasterNoDataValue(band, &hasNoData);
        std::vector<double> source(static_cast<std::size_t>(width)
                                   * static_cast<std::size_t>(height));
        if (GDALRasterIO(band, GF_Read, 0, 0, width, height,
                         source.data(), width, height, GDT_Float64, 0, 0) != CE_None) {
            setError(error, QStringLiteral("GEOTIFF_RASTER_READ_FAILED"));
            break;
        }

        const double centreLatitude = transform[3] + transform[5] * (height * 0.5);
        if (!std::isfinite(centreLatitude) || centreLatitude < -90.0
            || centreLatitude > 90.0 || transform[0] < -180.0
            || transform[0] + transform[1] * width > 180.0) {
            setError(error, QStringLiteral("GEOTIFF_GEOGRAPHIC_EXTENT_INVALID_OR_ANTIMERIDIAN_CROSSES"));
            break;
        }

        TerrainDemGrid decoded;
        decoded.horizontal_crs = "EPSG:4326";
        decoded.vertical_datum = "UNKNOWN";
        decoded.surface_type = "UNKNOWN";
        decoded.vertical_accuracy_model = "UNKNOWN";
        decoded.source_id = "UNATTESTED_GEOTIFF_IMPORT";
        decoded.source_version = "SHA256:" + sha256File(info.absoluteFilePath(), error).toStdString();
        if (decoded.source_version == "SHA256:") {
            break;
        }
        decoded.dataset_id = "UNATTESTED_FILE";
        decoded.dataset_version = "UNATTESTED";
        decoded.snapshot_id = "GEOTIFF-INSPECTION-" + decoded.source_version.substr(7);
        decoded.origin_longitude_deg = transform[0] + transform[1] * 0.5;
        decoded.origin_latitude_deg = transform[3] + transform[5] * (height - 0.5);
        decoded.latitude_step_deg = std::abs(transform[5]);
        decoded.longitude_step_deg = transform[1];
        const double metresPerLatitudeDegree = 111132.0;
        const double metresPerLongitudeDegree =
            111320.0 * std::max(0.0, std::cos(centreLatitude * 3.14159265358979323846 / 180.0));
        decoded.horizontal_resolution_m = std::sqrt(
            (decoded.latitude_step_deg * metresPerLatitudeDegree)
            * (decoded.longitude_step_deg * metresPerLongitudeDegree));
        decoded.vertical_accuracy_m = 0.0;
        decoded.rows = static_cast<std::size_t>(height);
        decoded.columns = static_cast<std::size_t>(width);
        decoded.elevation_m.resize(source.size());

        for (int sourceRow = 0; sourceRow < height; ++sourceRow) {
            const int destinationRow = height - 1 - sourceRow;
            for (int column = 0; column < width; ++column) {
                const std::size_t sourceIndex =
                    static_cast<std::size_t>(sourceRow) * width + column;
                const std::size_t destinationIndex =
                    static_cast<std::size_t>(destinationRow) * width + column;
                const double raw = source[sourceIndex];
                const bool isNoData = hasNoData
                    && ((std::isnan(noData) && std::isnan(raw)) || raw == noData);
                if (isNoData) {
                    decoded.elevation_m[destinationIndex] =
                        std::numeric_limits<double>::quiet_NaN();
                    continue;
                }
                if (!std::isfinite(raw)) {
                    setError(error, QStringLiteral("GEOTIFF_CONTAINS_NONFINITE_NON_NODATA_CELL"));
                    decoded.elevation_m.clear();
                    break;
                }
                const double elevation = raw * appliedScale + appliedOffset;
                if (!std::isfinite(elevation)) {
                    setError(error, QStringLiteral("GEOTIFF_DECODED_ELEVATION_NONFINITE"));
                    decoded.elevation_m.clear();
                    break;
                }
                decoded.elevation_m[destinationIndex] = elevation;
            }
            if (decoded.elevation_m.empty())
                break;
        }
        if (decoded.elevation_m.empty())
            break;

        *grid = std::move(decoded);
        if (error)
            error->clear();
        ok = true;
    } while (false);

    closeDataset();
    return ok;
}

} // namespace bluesky::planning
