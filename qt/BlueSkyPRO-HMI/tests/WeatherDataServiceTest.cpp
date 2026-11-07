#include "WeatherDataService.h"

#include <QFile>
#include <QTemporaryDir>
#include <QUrlQuery>

#include <cassert>

int main()
{
    QString error;
    const QUrl url = WeatherDataService::buildRequestUrl(
        QStringLiteral("20261107"), QStringLiteral("12"), 6,
        96.0, 5.0, 100.0, 10.0,
        {QStringLiteral("10"), QStringLiteral("50"), QStringLiteral("100")},
        &error);
    assert(url.isValid());
    assert(error.isEmpty());
    assert(url.host() == QStringLiteral("nomads.ncep.noaa.gov"));
    const QUrlQuery query(url);
    assert(query.queryItemValue(QStringLiteral("file")) == QStringLiteral("gfs.t12z.pgrb2.0p25.f006"));
    assert(query.queryItemValue(QStringLiteral("var_UGRD")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("var_VGRD")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("lev_10_m_above_ground")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("lev_50_m_above_ground")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("lev_100_m_above_ground")) == QStringLiteral("on"));
    assert(query.queryItemValue(QStringLiteral("dir")) == QStringLiteral("/gfs.20261107/12/atmos"));

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
                + QByteArrayLiteral("payload") + QByteArrayLiteral("7777"));
    valid.close();
    assert(WeatherDataService::isValidGrib2File(validPath));

    const QString invalidPath = temp.filePath(QStringLiteral("invalid.grib2"));
    QFile invalid(invalidPath);
    assert(invalid.open(QIODevice::WriteOnly));
    invalid.write(QByteArrayLiteral("<html>error</html>"));
    invalid.close();
    assert(!WeatherDataService::isValidGrib2File(invalidPath));

    return 0;
}
