#include "WeatherDataService.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>
#include <QVariantMap>

#include <algorithm>
#include <cmath>

namespace
{
const QString kEndpoint = QStringLiteral("https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p25.pl");
const QStringList kSupportedAglLevels{
    QStringLiteral("10"), QStringLiteral("20"), QStringLiteral("30"),
    QStringLiteral("40"), QStringLiteral("50"), QStringLiteral("80"),
    QStringLiteral("100"), QStringLiteral("1000")
};

void assignError(QString *target, const QString &message)
{
    if (target)
        *target = message;
}
}

WeatherDataService::WeatherDataService(QObject *parent)
    : QObject(parent),
      m_network(this)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_cacheDirectory = QDir(base).filePath(QStringLiteral("weather/noaa_gfs"));
    QDir().mkpath(m_cacheDirectory);
}

QString WeatherDataService::state() const { return m_state; }
QString WeatherDataService::lastError() const { return m_lastError; }
QString WeatherDataService::cacheDirectory() const { return m_cacheDirectory; }
QString WeatherDataService::lastDownloadedFile() const { return m_lastDownloadedFile; }

QUrl WeatherDataService::buildRequestUrl(const QString &dateUtc,
                                         const QString &cycleUtc,
                                         int forecastHour,
                                         double west,
                                         double south,
                                         double east,
                                         double north,
                                         const QStringList &levelsAglM,
                                         QString *error)
{
    const QDate date = QDate::fromString(dateUtc, QStringLiteral("yyyyMMdd"));
    if (!date.isValid() || date.toString(QStringLiteral("yyyyMMdd")) != dateUtc) {
        assignError(error, QStringLiteral("dateUtc must be a valid YYYYMMDD date"));
        return {};
    }
    if (cycleUtc != QLatin1String("00") && cycleUtc != QLatin1String("06")
        && cycleUtc != QLatin1String("12") && cycleUtc != QLatin1String("18")) {
        assignError(error, QStringLiteral("cycleUtc must be 00, 06, 12 or 18 UTC"));
        return {};
    }
    if (forecastHour < 0 || forecastHour > 384
        || (forecastHour > 120 && forecastHour % 3 != 0)) {
        assignError(error, QStringLiteral("forecastHour must be 0..120 hourly or a 3-hour step through 384"));
        return {};
    }
    const double values[] = {west, south, east, north};
    for (double value : values) {
        if (!std::isfinite(value)) {
            assignError(error, QStringLiteral("bounding box values must be finite"));
            return {};
        }
    }
    if (west < -180.0 || east > 180.0 || west >= east
        || south < -90.0 || north > 90.0 || south >= north) {
        assignError(error, QStringLiteral("invalid WGS84 bounding box; antimeridian-crossing boxes are not supported"));
        return {};
    }
    if (east - west > 30.0 || north - south > 30.0) {
        assignError(error, QStringLiteral("requested region exceeds the 30-degree safety limit"));
        return {};
    }
    if (levelsAglM.isEmpty()) {
        assignError(error, QStringLiteral("at least one AGL level is required"));
        return {};
    }

    QStringList levels;
    for (const QString &level : levelsAglM) {
        if (!kSupportedAglLevels.contains(level) || levels.contains(level)) {
            assignError(error, QStringLiteral("unsupported or duplicate AGL level: %1").arg(level));
            return {};
        }
        levels.append(level);
    }
    std::sort(levels.begin(), levels.end(), [](const QString &a, const QString &b) {
        return a.toInt() < b.toInt();
    });

    QUrl url(kEndpoint);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("file"),
                       QStringLiteral("gfs.t%1z.pgrb2.0p25.f%2")
                           .arg(cycleUtc).arg(forecastHour, 3, 10, QLatin1Char('0')));
    query.addQueryItem(QStringLiteral("var_UGRD"), QStringLiteral("on"));
    query.addQueryItem(QStringLiteral("var_VGRD"), QStringLiteral("on"));
    for (const QString &level : levels)
        query.addQueryItem(QStringLiteral("lev_%1_m_above_ground").arg(level), QStringLiteral("on"));
    query.addQueryItem(QStringLiteral("subregion"), QString());
    query.addQueryItem(QStringLiteral("leftlon"), QString::number(west, 'g', 12));
    query.addQueryItem(QStringLiteral("rightlon"), QString::number(east, 'g', 12));
    query.addQueryItem(QStringLiteral("toplat"), QString::number(north, 'g', 12));
    query.addQueryItem(QStringLiteral("bottomlat"), QString::number(south, 'g', 12));
    query.addQueryItem(QStringLiteral("dir"),
                       QStringLiteral("/gfs.%1/%2/atmos").arg(dateUtc, cycleUtc));
    url.setQuery(query);
    return url;
}

bool WeatherDataService::isValidGrib2File(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        assignError(error, QStringLiteral("cannot open downloaded file"));
        return false;
    }
    if (file.size() < 20) {
        assignError(error, QStringLiteral("GRIB2 payload is too small"));
        return false;
    }
    const QByteArray header = file.read(8);
    if (header.size() != 8 || header.left(4) != QByteArrayLiteral("GRIB")
        || static_cast<unsigned char>(header.at(7)) != 2) {
        assignError(error, QStringLiteral("response is not a GRIB edition 2 payload"));
        return false;
    }
    if (!file.seek(file.size() - 4) || file.read(4) != QByteArrayLiteral("7777")) {
        assignError(error, QStringLiteral("GRIB2 end marker is missing; payload may be truncated"));
        return false;
    }
    return true;
}

bool WeatherDataService::isReusableCacheEntry(const QString &filePath,
                                                     const QString &manifestPath,
                                                     const QString &expectedRequestUrl,
                                                     const QString &expectedModelRunUtc,
                                                     int expectedForecastHour,
                                                     QString *error)
{
    QString gribError;
    if (!isValidGrib2File(filePath, &gribError)) {
        assignError(error, QStringLiteral("cached GRIB2 file is invalid: %1").arg(gribError));
        return false;
    }

    QFile manifestFile(manifestPath);
    if (!manifestFile.open(QIODevice::ReadOnly)) {
        assignError(error, QStringLiteral("cached forecast manifest is missing or unreadable"));
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(manifestFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        assignError(error, QStringLiteral("cached forecast manifest is malformed"));
        return false;
    }
    const QJsonObject manifest = document.object();
    const QDateTime modelRun = QDateTime::fromString(expectedModelRunUtc, Qt::ISODate).toUTC();
    if (!modelRun.isValid() || expectedForecastHour < 0 || expectedForecastHour > 384) {
        assignError(error, QStringLiteral("expected forecast identity is invalid"));
        return false;
    }
    const QString expectedValidTime = modelRun.addSecs(qint64(expectedForecastHour) * 3600)
        .toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'"));
    QFile cachedFile(filePath);
    if (!cachedFile.open(QIODevice::ReadOnly)) {
        assignError(error, QStringLiteral("cached forecast file cannot be read"));
        return false;
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (!cachedFile.atEnd()) {
        const QByteArray chunk = cachedFile.read(1024 * 1024);
        if (chunk.isEmpty() && cachedFile.error() != QFile::NoError) {
            assignError(error, QStringLiteral("cached forecast file read failed"));
            return false;
        }
        hash.addData(chunk);
    }
    const QString actualHash = QString::fromLatin1(hash.result().toHex());
    const bool matches =
        manifest.value(QStringLiteral("cache_schema_version")).toInt() == 1
        && manifest.value(QStringLiteral("request_url")).toString() == expectedRequestUrl
        && manifest.value(QStringLiteral("model_run_utc")).toString() == expectedModelRunUtc
        && manifest.value(QStringLiteral("forecast_hour")).toInt(-1) == expectedForecastHour
        && manifest.value(QStringLiteral("forecast_valid_time_utc")).toString() == expectedValidTime
        && manifest.value(QStringLiteral("bytes")).toVariant().toLongLong() == cachedFile.size()
        && manifest.value(QStringLiteral("sha256")).toString() == actualHash
        && manifest.value(QStringLiteral("file")).toString() == QFileInfo(filePath).fileName();
    if (!matches) {
        assignError(error, QStringLiteral("cached forecast identity, valid time, size or SHA-256 does not match its manifest"));
        return false;
    }
    assignError(error, QString());
    return true;
}

bool WeatherDataService::requestWindSubset(const QString &dateUtc,
                                           const QString &cycleUtc,
                                           int forecastHour,
                                           double west,
                                           double south,
                                           double east,
                                           double north,
                                           const QStringList &levelsAglM)
{
    if (m_reply) {
        setError(QStringLiteral("another weather download is already active"));
        return false;
    }

    QString validationError;
    const QUrl url = buildRequestUrl(dateUtc, cycleUtc, forecastHour, west, south,
                                     east, north, levelsAglM, &validationError);
    if (!url.isValid() || url.isEmpty()) {
        setError(validationError);
        return false;
    }

    const QByteArray key = QCryptographicHash::hash(url.toEncoded(), QCryptographicHash::Sha256).toHex();
    const QString cycleDirectory = QDir(m_cacheDirectory).filePath(dateUtc + QLatin1Char('/') + cycleUtc);
    QDir().mkpath(cycleDirectory);
    const QString stem = QStringLiteral("gfs_f%1_%2").arg(forecastHour, 3, 10, QLatin1Char('0'))
                             .arg(QString::fromLatin1(key.left(16)));
    m_finalPath = QDir(cycleDirectory).filePath(stem + QStringLiteral(".grib2"));
    m_manifestPath = m_finalPath + QStringLiteral(".manifest.json");

    QString cachedError;
    const QDate modelRunDate = QDate::fromString(dateUtc, QStringLiteral("yyyyMMdd"));
    const QString modelRunUtc = modelRunDate.toString(QStringLiteral("yyyy-MM-dd"))
        + QLatin1Char('T') + cycleUtc + QStringLiteral(":00:00Z");
    if (QFileInfo::exists(m_finalPath) && QFileInfo::exists(m_manifestPath)
        && isReusableCacheEntry(m_finalPath, m_manifestPath, url.toString(),
                                modelRunUtc, forecastHour, &cachedError)) {
        QFile manifestFile(m_manifestPath);
        if (manifestFile.open(QIODevice::ReadOnly)) {
            QJsonObject manifest = QJsonDocument::fromJson(manifestFile.readAll()).object();
            manifest.insert(QStringLiteral("cache_reuse_status"),
                            QStringLiteral("VERIFIED_CONTENT_AND_FORECAST_VALID_TIME"));
            m_lastDownloadedFile = m_finalPath;
            emit lastDownloadedFileChanged();
            setError(QString());
            setState(QStringLiteral("CACHE_HIT"));
            emit forecastReady(m_finalPath, manifest.toVariantMap());
            return true;
        }
    }
    // A corrupt, incomplete or mismatched cache entry must never be reused.
    if (QFileInfo::exists(m_finalPath))
        QFile::remove(m_finalPath);
    if (QFileInfo::exists(m_manifestPath))
        QFile::remove(m_manifestPath);

    m_partialPath = m_finalPath + QStringLiteral(".part");
    m_partialFile = new QFile(m_partialPath, this);
    if (!m_partialFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        delete m_partialFile;
        m_partialFile = nullptr;
        setError(QStringLiteral("cannot create weather cache file"));
        return false;
    }

    m_receivedBytes = 0;
    m_hash.reset();
    const QDateTime modelRun = QDateTime::fromString(modelRunUtc, Qt::ISODate).toUTC();
    const QString forecastValidTimeUtc = modelRun.addSecs(qint64(forecastHour) * 3600)
        .toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'"));
    m_manifest = {
        {QStringLiteral("cache_schema_version"), 1},
        {QStringLiteral("cache_reuse_policy"), QStringLiteral("IMMUTABLE_PRODUCT_WITH_EXACT_VALID_TIME_AND_SHA256")},
        {QStringLiteral("forecast_time_semantics"), QStringLiteral("VALID_TIME_INSTANT")},
        {QStringLiteral("forecast_valid_time_utc"), forecastValidTimeUtc},
        {QStringLiteral("classification"), QStringLiteral("LIVE_MODEL_DATA_UNDECODED")},
        {QStringLiteral("provider"), QStringLiteral("NOAA/NCEP NOMADS")},
        {QStringLiteral("model"), QStringLiteral("GFS 0.25 degree")},
        {QStringLiteral("product"), QStringLiteral("GRIB2 regional wind subset")},
        {QStringLiteral("model_run_utc"), dateUtc + QLatin1Char('T') + cycleUtc + QStringLiteral(":00:00Z")},
        {QStringLiteral("forecast_hour"), forecastHour},
        {QStringLiteral("bbox_wgs84_west_south_east_north"), QJsonArray{west, south, east, north}},
        {QStringLiteral("variables"), QJsonArray{QStringLiteral("UGRD"), QStringLiteral("VGRD")}},
        {QStringLiteral("levels_m_agl"), QJsonArray::fromStringList(levelsAglM)},
        {QStringLiteral("request_url"), url.toString()},
        {QStringLiteral("decode_status"), QStringLiteral("NOT_PERFORMED")},
        {QStringLiteral("optimizer_use_status"), QStringLiteral("BLOCKED_UNTIL_DECODED_AND_QC_PASSED")}
    };

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("BlueSkyPRO-WeatherDataService/0.1"));
    request.setRawHeader("Accept", "application/octet-stream");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_network.get(request);
    connect(m_reply, &QIODevice::readyRead, this, &WeatherDataService::consumeReplyData);
    connect(m_reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) { emit progressChanged(received, total); });
    connect(m_reply, &QNetworkReply::finished, this, &WeatherDataService::finishReply);
    setError(QString());
    setState(QStringLiteral("DOWNLOADING"));
    return true;
}

void WeatherDataService::consumeReplyData()
{
    if (!m_reply || !m_partialFile)
        return;
    const QByteArray chunk = m_reply->readAll();
    if (chunk.isEmpty())
        return;
    m_receivedBytes += chunk.size();
    if (m_receivedBytes > kMaximumDownloadBytes) {
        QPointer<QNetworkReply> reply = m_reply;
        failDownload(QStringLiteral("weather subset exceeds the 256 MiB download limit"));
        if (reply)
            reply->abort();
        return;
    }
    if (m_partialFile->write(chunk) != chunk.size()) {
        QPointer<QNetworkReply> reply = m_reply;
        failDownload(QStringLiteral("failed to write weather cache"));
        if (reply)
            reply->abort();
        return;
    }
    m_hash.addData(chunk);
}

void WeatherDataService::finishReply()
{
    if (!m_reply)
        return;
    consumeReplyData();
    if (!m_reply)
        return;
    const QNetworkReply::NetworkError networkError = m_reply->error();
    const QString networkErrorText = m_reply->errorString();
    m_reply->deleteLater();
    m_reply = nullptr;
    if (networkError != QNetworkReply::NoError) {
        failDownload(QStringLiteral("NOAA request failed: %1").arg(networkErrorText));
        return;
    }
    if (!m_partialFile || !m_partialFile->flush()) {
        failDownload(QStringLiteral("failed to flush downloaded weather file"));
        return;
    }
    m_partialFile->close();
    QString validationError;
    if (!isValidGrib2File(m_partialPath, &validationError)) {
        failDownload(QStringLiteral("NOAA response rejected: %1").arg(validationError));
        return;
    }

    m_manifest.insert(QStringLiteral("retrieved_at_utc"),
                      QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    m_manifest.insert(QStringLiteral("file"), QFileInfo(m_finalPath).fileName());
    m_manifest.insert(QStringLiteral("bytes"), static_cast<double>(m_receivedBytes));
    m_manifest.insert(QStringLiteral("sha256"), QString::fromLatin1(m_hash.result().toHex()));
    if (QFileInfo::exists(m_finalPath))
        QFile::remove(m_finalPath);
    if (!QFile::rename(m_partialPath, m_finalPath)) {
        failDownload(QStringLiteral("cannot atomically publish downloaded GRIB2 file"));
        return;
    }
    QString manifestError;
    if (!writeManifest(m_manifest, &manifestError)) {
        QFile::remove(m_finalPath);
        failDownload(manifestError);
        return;
    }

    m_lastDownloadedFile = m_finalPath;
    emit lastDownloadedFileChanged();
    if (m_partialFile) {
        m_partialFile->deleteLater();
        m_partialFile = nullptr;
    }
    setState(QStringLiteral("READY_UNDECODED"));
    emit forecastReady(m_finalPath, m_manifest.toVariantMap());
}

bool WeatherDataService::writeManifest(const QJsonObject &manifest, QString *error)
{
    QSaveFile file(m_manifestPath);
    if (!file.open(QIODevice::WriteOnly)) {
        assignError(error, QStringLiteral("cannot create weather manifest"));
        return false;
    }
    file.write(QJsonDocument(manifest).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        assignError(error, QStringLiteral("cannot atomically publish weather manifest"));
        return false;
    }
    return true;
}

void WeatherDataService::failDownload(const QString &reason)
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_partialFile) {
        m_partialFile->close();
        m_partialFile->deleteLater();
        m_partialFile = nullptr;
    }
    if (!m_partialPath.isEmpty())
        QFile::remove(m_partialPath);
    setError(reason);
    setState(QStringLiteral("ERROR"));
    emit downloadFailed(reason);
}

void WeatherDataService::cancelPending()
{
    if (!m_reply)
        return;
    QPointer<QNetworkReply> reply = m_reply;
    failDownload(QStringLiteral("weather download cancelled"));
    if (reply)
        reply->abort();
}

void WeatherDataService::setState(const QString &value)
{
    if (m_state == value)
        return;
    m_state = value;
    emit stateChanged();
}

void WeatherDataService::setError(const QString &value)
{
    if (m_lastError == value)
        return;
    m_lastError = value;
    emit lastErrorChanged();
}
