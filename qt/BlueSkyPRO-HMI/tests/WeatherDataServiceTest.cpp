#include "WeatherDataService.h"

#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <QUrlQuery>

#include <cassert>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("BlueSky PRO Test"));
    QCoreApplication::setApplicationName(QStringLiteral("WeatherDataServiceTest"));
    WeatherDataService batchService;
    assert(!batchService.requestWindSubsetBatch(
        QStringLiteral("20261107"), QStringLiteral("12"), QVariantList{},
        96.0, 5.0, 100.0, 10.0, {QStringLiteral("50")}));
    assert(!batchService.requestWindSubsetBatch(
        QStringLiteral("20261107"), QStringLiteral("12"), QVariantList{6, 6},
        96.0, 5.0, 100.0, 10.0, {QStringLiteral("50")}));
    assert(!batchService.requestWindSubsetBatch(
        QStringLiteral("20261107"), QStringLiteral("12"), QVariantList{6, 121},
        96.0, 5.0, 100.0, 10.0, {QStringLiteral("50")}));
    assert(!batchService.requestWindSubsetBatch(
        QStringLiteral("20261107"), QStringLiteral("12"), QVariantList{6.5},
        96.0, 5.0, 100.0, 10.0, {QStringLiteral("50")}));

    QString error;
    const QUrl url = WeatherDataService::buildRequestUrl(
        QStringLiteral("20261107"), QStringLiteral("12"), 6,
        96.0, 5.0, 100.0, 10.0,
        {QStringLiteral("2"), QStringLiteral("10"), QStringLiteral("50"), QStringLiteral("100")},
        &error);
    assert(url.isValid());
    assert(error.isEmpty());
    assert(url.host() == QStringLiteral("nomads.ncep.noaa.gov"));
    const QUrlQuery query(url);
    assert(query.queryItemValue(QStringLiteral("file")) == QStringLiteral("gfs.t12z.pgrb2.0p25.f006"));
    assert(query.queryItemValue(QStringLiteral("var_UGRD")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("var_VGRD")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("lev_2_m_above_ground")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("lev_10_m_above_ground")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("lev_50_m_above_ground")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("lev_100_m_above_ground")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("dir")) == QStringLiteral("/gfs.20261107/12/atmos"));

    // Parameter order must not create duplicate downloads for the same forecast subset.
    const QUrl sameRequestUrl = WeatherDataService::buildRequestUrl(
        QStringLiteral("20261107"), QStringLiteral("12"), 6,
        96.0, 5.0, 100.0, 10.0,
        {QStringLiteral("100"), QStringLiteral("10"), QStringLiteral("50"), QStringLiteral("2")});
    assert(WeatherDataService::isSameRequestIdentity(url, sameRequestUrl));
    const QUrl differentRequestUrl = WeatherDataService::buildRequestUrl(
        QStringLiteral("20261107"), QStringLiteral("12"), 6,
        97.0, 5.0, 100.0, 10.0, {QStringLiteral("10"), QStringLiteral("50"), QStringLiteral("100"), QStringLiteral("2")});
    assert(!WeatherDataService::isSameRequestIdentity(url, differentRequestUrl));
    assert(!WeatherDataService::isSameRequestIdentity(QUrl(), url));

    error.clear();
    assert(WeatherDataService::buildRequestUrl(
        QStringLiteral("20261107"), QStringLiteral("12"), 121,
        96.0, 5.0, 100.0, 10.0, {QStringLiteral("50")}, &error).isEmpty());
    assert(!error.isEmpty());

    error.clear();
    assert(WeatherDataService::buildRequestUrl(
        QStringLiteral("20261107"), QStringLiteral("12"), 6,
        96.0, 5.0, 100.0, 10.0, {QStringLiteral("200")}, &error).isEmpty());
    assert(!error.isEmpty());

    QTemporaryDir temp;
    assert(temp.isValid());
    const QString validPath = temp.filePath(QStringLiteral("valid.grib2"));
    QFile valid(validPath);
    assert(valid.open(QIODevice::WriteOnly));
    valid.write(QByteArrayLiteral("GRIB") + QByteArray(3, char(0)) + QByteArray(1, char(2))
                + QByteArrayLiteral("payload0") + QByteArrayLiteral("7777"));
    valid.close();
    assert(WeatherDataService::isValidGrib2File(validPath));

    const QString manifestPath = temp.filePath(QStringLiteral("valid.manifest.json"));
    QFile manifestFile(manifestPath);
    assert(manifestFile.open(QIODevice::WriteOnly));
    const QByteArray validBytes = [&]() {
        QFile input(validPath);
        assert(input.open(QIODevice::ReadOnly));
        return input.readAll();
    }();
    const QString validHash = QString::fromLatin1(
        QCryptographicHash::hash(validBytes, QCryptographicHash::Sha256).toHex());
    const QByteArray manifest = QStringLiteral(
        R"({"cache_schema_version":1,"request_url":"https://example.test/gfs","model_run_utc":"2026-11-07T12:00:00Z","forecast_hour":6,"forecast_valid_time_utc":"2026-11-07T18:00:00Z","bytes":%1,"sha256":"%2","file":"valid.grib2"})")
        .arg(validBytes.size()).arg(validHash).toUtf8();
    manifestFile.write(manifest);
    manifestFile.close();
    assert(WeatherDataService::isReusableCacheEntry(
        validPath, manifestPath, QStringLiteral("https://example.test/gfs"),
        QStringLiteral("2026-11-07T12:00:00Z"), 6, &error));
    assert(error.isEmpty());
    assert(!WeatherDataService::isReusableCacheEntry(
        validPath, manifestPath, QStringLiteral("https://example.test/gfs"),
        QStringLiteral("2026-11-07T12:00:00Z"), 9, &error));
    assert(!error.isEmpty());

    // Same URL and forecast lead are not sufficient if the cached bytes were altered.
    QFile tampered(validPath);
    assert(tampered.open(QIODevice::ReadWrite));
    assert(tampered.seek(8));
    tampered.write("X");
    tampered.close();
    assert(!WeatherDataService::isReusableCacheEntry(
        validPath, manifestPath, QStringLiteral("https://example.test/gfs"),
        QStringLiteral("2026-11-07T12:00:00Z"), 6, &error));

    const QString invalidPath = temp.filePath(QStringLiteral("invalid.grib2"));
    QFile invalid(invalidPath);
    assert(invalid.open(QIODevice::WriteOnly));
    invalid.write(QByteArrayLiteral("<html>error</html>"));
    invalid.close();
    assert(!WeatherDataService::isValidGrib2File(invalidPath));

    return 0;
}
