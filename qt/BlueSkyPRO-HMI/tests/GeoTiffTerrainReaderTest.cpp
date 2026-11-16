#include "GeoTiffTerrainReader.h"

#include <gdal.h>
#include <cpl_conv.h>
#include <ogr_srs_api.h>

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QTemporaryDir>

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

using namespace bluesky::planning;

static QString createFixture(const QString &path, const char *crs = "EPSG:4326",
                             bool rotated = false, bool nodata = false)
{
    GDALAllRegister();
    GDALDriverH driver = GDALGetDriverByName("GTiff");
    assert(driver);
    const QByteArray encodedPath = QFile::encodeName(path);
    GDALDatasetH dataset = GDALCreate(driver, encodedPath.constData(), 2, 2, 1,
                                      GDT_Float32, nullptr);
    assert(dataset);
    double transform[6] = {10.0, 0.01, rotated ? 0.001 : 0.0,
                           50.0, 0.0, -0.01};
    assert(GDALSetGeoTransform(dataset, transform) == CE_None);
    OGRSpatialReferenceH srs = OSRNewSpatialReference(nullptr);
    assert(OSRSetFromUserInput(srs, crs) == OGRERR_NONE);
    char *wkt = nullptr;
    assert(OSRExportToWkt(srs, &wkt) == OGRERR_NONE);
    assert(GDALSetProjection(dataset, wkt) == CE_None);
    CPLFree(wkt);
    OSRDestroySpatialReference(srs);

    GDALRasterBandH band = GDALGetRasterBand(dataset, 1);
    GDALSetRasterUnitType(band, "m");
    GDALSetRasterScale(band, 2.0);
    GDALSetRasterOffset(band, 10.0);
    const float values[4] = {1.0F, 2.0F, 3.0F, 4.0F};
    assert(GDALRasterIO(band, GF_Write, 0, 0, 2, 2,
                        const_cast<float *>(values), 2, 2, GDT_Float32, 0, 0) == CE_None);
    if (nodata) {
        GDALSetRasterNoDataValue(band, 4.0);
    }
    GDALClose(dataset);
    return path;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir temporary;
    assert(temporary.isValid());

    const QString validPath = createFixture(QDir(temporary.path()).filePath("valid.tif"));
    TerrainDemGrid grid;
    QString error;
    assert(GeoTiffTerrainReader::readForInspection(validPath, &grid, &error));
    assert(error.isEmpty());
    assert(grid.horizontal_crs == "EPSG:4326");
    assert(grid.rows == 2 && grid.columns == 2);
    assert(std::abs(grid.origin_longitude_deg - 10.005) < 1e-9);
    assert(std::abs(grid.origin_latitude_deg - 49.985) < 1e-9);
    // Source rows are north-to-south; planning grid row zero is south-to-north.
    assert(std::abs(grid.elevation_m[0] - 16.0) < 1e-6);
    assert(std::abs(grid.elevation_m[1] - 18.0) < 1e-6);
    assert(std::abs(grid.elevation_m[2] - 12.0) < 1e-6);
    assert(std::abs(grid.elevation_m[3] - 14.0) < 1e-6);
    // Import is deliberately not safety eligible without trusted registry metadata.
    assert(grid.surface_type == "UNKNOWN");
    assert(grid.vertical_datum == "UNKNOWN");
    assert(grid.vertical_accuracy_model == "UNKNOWN");
    assert(grid.vertical_accuracy_m == 0.0);
    assert(grid.source_id == "UNATTESTED_GEOTIFF_IMPORT");
    assert(grid.source_version.rfind("SHA256:", 0) == 0);

    const QString noDataPath = createFixture(
        QDir(temporary.path()).filePath("nodata.tif"), "EPSG:4326", false, true);
    TerrainDemGrid noDataGrid;
    assert(GeoTiffTerrainReader::readForInspection(noDataPath, &noDataGrid, &error));
    assert(std::isnan(noDataGrid.elevation_m[1]));

    const QString wrongCrsPath = createFixture(
        QDir(temporary.path()).filePath("wrong-crs.tif"), "EPSG:3857");
    TerrainDemGrid rejected;
    assert(!GeoTiffTerrainReader::readForInspection(wrongCrsPath, &rejected, &error));
    assert(error == "GEOTIFF_HORIZONTAL_CRS_MUST_BE_EPSG_4326");

    const QString rotatedPath = createFixture(
        QDir(temporary.path()).filePath("rotated.tif"), "EPSG:4326", true);
    assert(!GeoTiffTerrainReader::readForInspection(rotatedPath, &rejected, &error));
    assert(error == "GEOTIFF_MUST_BE_REGULAR_NORTH_UP_UNROTATED");

    assert(!GeoTiffTerrainReader::readForInspection(validPath, nullptr, &error));
    assert(error == "GEOTIFF_OUTPUT_GRID_REQUIRED");
    return 0;
}
