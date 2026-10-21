#include "FaaNmsClient.h"

#include "CredentialVault.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QTimer>
#include <QJsonArray>
#include <QDateTime>
#include <QUuid>
#include <QVariantMap>
#include <cmath>

FaaNmsClient::FaaNmsClient(CredentialVault *vault, QObject *parent)
    : QObject(parent), m_vault(vault), m_network(new QNetworkAccessManager(this))
{
}

QString FaaNmsClient::status() const { return m_status; }
QString FaaNmsClient::lastError() const { return m_lastError; }
QString FaaNmsClient::environment() const { return m_environment; }
QVariantList FaaNmsClient::notamRecords() const { return m_notamRecords; }
QString FaaNmsClient::notamSnapshotId() const { return m_notamSnapshotId; }

void FaaNmsClient::setEnvironment(const QString &environment)
{
    const QString normalized = environment.trimmed().toUpper();
    if (normalized != QStringLiteral("STAGING") && normalized != QStringLiteral("FIT")) {
        setLastError(QStringLiteral("Only FAA NMS STAGING and FIT environments are enabled for this test client."));
        return;
    }
    if (m_environment == normalized) return;
    m_environment = normalized;
    emit environmentChanged();
}

QUrl FaaNmsClient::baseUrl() const
{
    if (m_environment == QStringLiteral("FIT"))
        return QUrl(QStringLiteral("https://api-fit.cgifederal-aim.com/"));
    return QUrl(QStringLiteral("https://api-staging.cgifederal-aim.com/"));
}

void FaaNmsClient::setStatus(const QString &value)
{
    if (m_status == value) return;
    m_status = value;
    emit statusChanged();
}

void FaaNmsClient::setLastError(const QString &value)
{
    if (m_lastError == value) return;
    m_lastError = value;
    emit lastErrorChanged();
}

void FaaNmsClient::finishFailure(const QString &state, const QString &message)
{
    m_requestInProgress = false;
    setStatus(state);
    setLastError(message);
    emit connectionTestFinished(false, message);
}

void FaaNmsClient::testConnection()
{
    if (m_requestInProgress) return;
    if (!m_vault) {
        finishFailure(QStringLiteral("ERROR"), QStringLiteral("Credential vault is unavailable."));
        return;
    }

    const QString clientId = m_vault->readSecret(QStringLiteral("faa_nms_client_id"));
    const QString clientSecret = m_vault->readSecret(QStringLiteral("faa_nms_client_secret"));
    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        finishFailure(QStringLiteral("NOT_CONFIGURED"),
                      QStringLiteral("FAA NMS staging credentials are not configured in the Windows Credential Manager."));
        return;
    }

    m_requestInProgress = true;
    setStatus(QStringLiteral("CONNECTING"));
    setLastError({});

    QUrl url = baseUrl();
    url.setPath(QStringLiteral("/v1/auth/token"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));
    QByteArray basic = clientId.toUtf8() + ':' + clientSecret.toUtf8();
    request.setRawHeader("Authorization", "Basic " + basic.toBase64());
    basic.fill('\0');

    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("client_credentials"));
    QNetworkReply *reply = m_network->post(request, form.toString(QUrl::FullyEncoded).toUtf8());
    QTimer *timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, reply, [reply]() {
        reply->setProperty("blueskyTimedOut", true);
        reply->abort();
    });
    timeout->start(15000);

    connect(reply, &QNetworkReply::finished, this, [this, reply, timeout]() {
        timeout->stop();
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QByteArray body = reply->readAll();
        reply->deleteLater();

        if (networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300) {
            const bool timedOut = reply->property("blueskyTimedOut").toBool();
            const QString state = timedOut ? QStringLiteral("TIMEOUT")
                : ((httpStatus == 401 || httpStatus == 403)
                ? QStringLiteral("AUTH_FAILED")
                : (httpStatus == 429 ? QStringLiteral("RATE_LIMITED")
                                     : (networkError == QNetworkReply::TimeoutError
                                            ? QStringLiteral("TIMEOUT") : QStringLiteral("ERROR"))));
            finishFailure(state, QStringLiteral("FAA NMS token request failed (HTTP %1, network error %2).")
                                     .arg(httpStatus).arg(static_cast<int>(networkError)));
            return;
        }

        QJsonParseError parseError {};
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        const QJsonObject object = document.object();
        const QString accessToken = object.value(QStringLiteral("access_token")).toString();
        const QJsonValue expiresValue = object.value(QStringLiteral("expires_in"));
        // FAA NMS examples encode expires_in as a JSON string ("1799"),
        // although some OAuth providers return a JSON number. Accept both.
        bool expiresOk = false;
        const int expiresIn = expiresValue.isString()
            ? expiresValue.toString().toInt(&expiresOk)
            : (expiresValue.isDouble()
                   ? (expiresOk = true, expiresValue.toInt())
                   : 0);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()
            || accessToken.isEmpty() || !expiresOk || expiresIn <= 0) {
            QString providerError = object.value(QStringLiteral("error_description")).toString();
            if (providerError.isEmpty())
                providerError = object.value(QStringLiteral("error")).toString();
            if (providerError.isEmpty())
                providerError = object.value(QStringLiteral("message")).toString();
            providerError = providerError.left(180).replace('\n', ' ').replace('\r', ' ');
            const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
            QString detail = QStringLiteral("FAA NMS token response is invalid (HTTP %1; content-type: %2; JSON parse: %3).")
                                 .arg(httpStatus)
                                 .arg(contentType.isEmpty() ? QStringLiteral("missing") : contentType)
                                 .arg(parseError.error == QJsonParseError::NoError
                                          ? QStringLiteral("valid JSON but required fields are missing")
                                          : parseError.errorString());
            if (!providerError.isEmpty())
                detail += QStringLiteral(" Provider message: ") + providerError;
            finishFailure(QStringLiteral("INVALID_RESPONSE"), detail);
            return;
        }
        requestPing(accessToken);
    });
}

void FaaNmsClient::queryNotams(double latitude, double longitude, double radiusNm)
{
    if (m_queryInProgress || m_requestInProgress) return;
    if (!std::isfinite(latitude) || latitude < -90.0 || latitude > 90.0
        || !std::isfinite(longitude) || longitude < -180.0 || longitude > 180.0
        || !std::isfinite(radiusNm) || radiusNm <= 0.0 || radiusNm > 100.0) {
        emit notamQueryFinished(false, QStringLiteral("Invalid query: latitude/longitude out of range or radius must be >0 and <=100 NM."));
        return;
    }
    if (!m_vault) {
        emit notamQueryFinished(false, QStringLiteral("Credential vault is unavailable."));
        return;
    }
    const QString clientId = m_vault->readSecret(QStringLiteral("faa_nms_client_id"));
    const QString clientSecret = m_vault->readSecret(QStringLiteral("faa_nms_client_secret"));
    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        emit notamQueryFinished(false, QStringLiteral("FAA NMS credentials are not configured."));
        return;
    }

    m_queryInProgress = true;
    QUrl tokenUrl = baseUrl();
    tokenUrl.setPath(QStringLiteral("/v1/auth/token"));
    QNetworkRequest tokenRequest(tokenUrl);
    tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                           QStringLiteral("application/x-www-form-urlencoded"));
    QByteArray basic = clientId.toUtf8() + ':' + clientSecret.toUtf8();
    tokenRequest.setRawHeader("Authorization", "Basic " + basic.toBase64());
    basic.fill('\0');
    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("client_credentials"));
    QNetworkReply *tokenReply = m_network->post(tokenRequest, form.toString(QUrl::FullyEncoded).toUtf8());
    QTimer *tokenTimeout = new QTimer(tokenReply);
    tokenTimeout->setSingleShot(true);
    connect(tokenTimeout, &QTimer::timeout, tokenReply, [tokenReply]() {
        tokenReply->setProperty("blueskyTimedOut", true);
        tokenReply->abort();
    });
    tokenTimeout->start(15000);
    connect(tokenReply, &QNetworkReply::finished, this,
            [this, tokenReply, tokenTimeout, latitude, longitude, radiusNm]() {
        tokenTimeout->stop();
        const int http = tokenReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto networkError = tokenReply->error();
        const QByteArray body = tokenReply->readAll();
        const bool timedOut = tokenReply->property("blueskyTimedOut").toBool();
        tokenReply->deleteLater();
        if (networkError != QNetworkReply::NoError || http < 200 || http >= 300) {
            m_queryInProgress = false;
            emit notamQueryFinished(false, QStringLiteral("FAA NMS OAuth failed (HTTP %1, network error %2%3).")
                .arg(http).arg(static_cast<int>(networkError))
                .arg(timedOut ? QStringLiteral(", timeout") : QString()));
            return;
        }
        QJsonParseError parseError {};
        const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
        const QJsonObject obj = doc.object();
        const QString token = obj.value(QStringLiteral("access_token")).toString();
        const QJsonValue expiry = obj.value(QStringLiteral("expires_in"));
        bool expiryOk = false;
        const int expirySeconds = expiry.isString() ? expiry.toString().toInt(&expiryOk)
            : (expiry.isDouble() ? (expiryOk = true, expiry.toInt()) : 0);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()
            || token.isEmpty() || !expiryOk || expirySeconds <= 0) {
            m_queryInProgress = false;
            emit notamQueryFinished(false, QStringLiteral("FAA NMS OAuth response did not match documented token schema."));
            return;
        }
        requestNotams(token, latitude, longitude, radiusNm);
    });
}

void FaaNmsClient::requestNotams(const QString &accessToken, double latitude,
                                 double longitude, double radiusNm)
{
    QUrl url = baseUrl();
    url.setPath(QStringLiteral("/nmsapi/v1/notams"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("latitude"), QString::number(latitude, 'f', 6));
    query.addQueryItem(QStringLiteral("longitude"), QString::number(longitude, 'f', 6));
    query.addQueryItem(QStringLiteral("radius"), QString::number(radiusNm, 'f', 2));
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
    request.setRawHeader("nmsResponseFormat", "GEOJSON");
    QNetworkReply *reply = m_network->get(request);
    QTimer *timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, reply, [reply]() {
        reply->setProperty("blueskyTimedOut", true);
        reply->abort();
    });
    timeout->start(15000);
    connect(reply, &QNetworkReply::finished, this, [this, reply, timeout]() {
        timeout->stop();
        const int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto networkError = reply->error();
        const QByteArray body = reply->readAll();
        const bool timedOut = reply->property("blueskyTimedOut").toBool();
        const QString requestId = reply->rawHeader("x-request-id").left(100);
        reply->deleteLater();
        m_queryInProgress = false;
        if (networkError != QNetworkReply::NoError || http < 200 || http >= 300) {
            emit notamQueryFinished(false, QStringLiteral("FAA NMS NOTAM query failed (HTTP %1, network error %2%3%4).")
                .arg(http).arg(static_cast<int>(networkError))
                .arg(timedOut ? QStringLiteral(", timeout") : QString())
                .arg(requestId.isEmpty() ? QString() : QStringLiteral(", request-id: ") + requestId));
            return;
        }
        QJsonParseError parseError {};
        const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            emit notamQueryFinished(false, QStringLiteral("FAA NMS NOTAM response is not a valid JSON object."));
            return;
        }
        const QJsonObject root = doc.object();
        const QJsonValue statusValue = root.value(QStringLiteral("status"));
        const QJsonValue dataValue = root.value(QStringLiteral("data"));
        if (!root.contains(QStringLiteral("status")) || statusValue.toString().compare(QStringLiteral("Success"), Qt::CaseInsensitive) != 0) {
            QString statusText = statusValue.isString() ? statusValue.toString() : QStringLiteral("missing/unsupported");
            emit notamQueryFinished(false, QStringLiteral("FAA NMS reported a missing or non-success response status (%1). No records accepted.")
                .arg(statusText.left(80)));
            return;
        }

        // The staging API has returned the documented {status, data} envelope
        // with GeoJSON represented as a FeatureCollection object, not an array.
        // Accept only recognized GeoJSON containers; never treat an unknown
        // payload as an empty airspace result.
        QJsonValue payloadValue = dataValue;
        // Some staging deployments serialize the "data" payload as a JSON
        // string inside the outer JSON envelope. Decode it before inspecting
        // GeoJSON structure; keep rejecting unrecognized payloads.
        if (payloadValue.isString()) {
            QJsonParseError nestedError {};
            const QByteArray nestedBytes = payloadValue.toString().toUtf8();
            const QJsonDocument nestedDoc = QJsonDocument::fromJson(nestedBytes, &nestedError);
            if (nestedError.error == QJsonParseError::NoError && !nestedDoc.isNull()) {
                if (nestedDoc.isObject()) payloadValue = nestedDoc.object();
                else if (nestedDoc.isArray()) payloadValue = nestedDoc.array();
            }
        }

        const QJsonObject dataObject = payloadValue.toObject();
        QJsonValue geojsonValue = dataObject.value(QStringLiteral("geojson"));
        if (geojsonValue.isUndefined()) geojsonValue = dataObject.value(QStringLiteral("geoJson"));
        if (geojsonValue.isUndefined()) geojsonValue = dataObject.value(QStringLiteral("features"));

        QJsonArray records;
        if (geojsonValue.isArray()) {
            records = geojsonValue.toArray();
        } else if (geojsonValue.isObject()) {
            const QJsonObject geojsonObject = geojsonValue.toObject();
            if (geojsonObject.value(QStringLiteral("type")).toString() == QStringLiteral("FeatureCollection")
                && geojsonObject.value(QStringLiteral("features")).isArray()) {
                records = geojsonObject.value(QStringLiteral("features")).toArray();
            } else if (geojsonObject.value(QStringLiteral("type")).toString() == QStringLiteral("Feature")) {
                records.append(geojsonObject);
            }
        } else if (dataObject.value(QStringLiteral("type")).toString() == QStringLiteral("FeatureCollection")
                   && dataObject.value(QStringLiteral("features")).isArray()) {
            records = dataObject.value(QStringLiteral("features")).toArray();
        } else if (payloadValue.isArray()) {
            records = payloadValue.toArray();
        }

        // The FAA staging endpoint may return {status:"Success", data:{}}
        // when the requested radius has no matching records. Treat this as a
        // successful empty result, not as a schema failure. This does NOT mean
        // that the airspace is unrestricted; FAA NMS is a US test source.
        const bool recognizedEmptyResult = payloadValue.isObject() && dataObject.isEmpty();
        const bool recognizedGeoJson = recognizedEmptyResult
            || geojsonValue.isArray()
            || (geojsonValue.isObject()
                && (geojsonValue.toObject().value(QStringLiteral("type")).toString() == QStringLiteral("FeatureCollection")
                    || geojsonValue.toObject().value(QStringLiteral("type")).toString() == QStringLiteral("Feature")))
            || (dataObject.value(QStringLiteral("type")).toString() == QStringLiteral("FeatureCollection")
                && dataObject.value(QStringLiteral("features")).isArray())
            || payloadValue.isArray();
        if (!recognizedGeoJson) {
            QStringList keys = root.keys();
            keys.sort();
            const QString dataKeys = dataObject.keys().join(QStringLiteral(", ")).left(160);
            const QString payloadType = dataValue.isString() ? QStringLiteral("string")
                : dataValue.isObject() ? QStringLiteral("object")
                : dataValue.isArray() ? QStringLiteral("array")
                : dataValue.isNull() ? QStringLiteral("null") : QStringLiteral("other");
            emit notamQueryFinished(false, QStringLiteral("FAA NMS response schema mismatch; top-level keys: %1; data type: %2; data keys: %3. No data accepted.")
                .arg(keys.join(QStringLiteral(", ")).left(100), payloadType,
                     dataKeys.isEmpty() ? QStringLiteral("none") : dataKeys));
            return;
        }
        QVariantList normalizedRecords;
        int featuresWithGeometry = 0;
        for (const QJsonValue &value : records) {
            if (!value.isObject()) continue;
            const QJsonObject feature = value.toObject();
            if (feature.value(QStringLiteral("type")).toString() != QStringLiteral("Feature")) continue;
            const QJsonObject properties = feature.value(QStringLiteral("properties")).toObject();
            const QJsonObject core = properties.value(QStringLiteral("coreNOTAMData")).toObject();
            const QJsonObject notam = core.value(QStringLiteral("notam")).toObject();
            const QString id = notam.value(QStringLiteral("id")).toString();
            if (id.isEmpty()) continue;

            QVariantMap record;
            record.insert(QStringLiteral("id"), id);
            record.insert(QStringLiteral("number"), notam.value(QStringLiteral("number")).toString());
            record.insert(QStringLiteral("location"), notam.value(QStringLiteral("location")).toString());
            record.insert(QStringLiteral("source"), QStringLiteral("FAA NMS — US TEST SOURCE"));
            record.insert(QStringLiteral("environment"), m_environment);
            record.insert(QStringLiteral("effectiveStart"), notam.value(QStringLiteral("effectiveStart")).toString());
            record.insert(QStringLiteral("effectiveEnd"), notam.value(QStringLiteral("effectiveEnd")).toString());
            record.insert(QStringLiteral("lowerLimitRaw"), notam.value(QStringLiteral("lowerLimit")).toString());
            record.insert(QStringLiteral("upperLimitRaw"), notam.value(QStringLiteral("upperLimit")).toString());
            // FAA limits such as SFC, 280M, and FL values require explicit conversion rules.
            // Preserve raw limits; do not silently assign AGL/AMSL or numeric zero.
            record.insert(QStringLiteral("altitudeReference"), QStringLiteral("UNKNOWN"));
            record.insert(QStringLiteral("text"), notam.value(QStringLiteral("text")).toString());
            QJsonArray translations = core.value(QStringLiteral("notamTranslation")).toArray();
            QStringList originalTexts;
            for (const QJsonValue &translationValue : translations) {
                const QJsonObject translation = translationValue.toObject();
                const QString formatted = translation.value(QStringLiteral("formattedText")).toString();
                const QString simple = translation.value(QStringLiteral("simpleText")).toString();
                if (!formatted.isEmpty()) originalTexts.append(formatted);
                else if (!simple.isEmpty()) originalTexts.append(simple);
            }
            record.insert(QStringLiteral("originalText"), originalTexts.join(QStringLiteral("\\n\\n")));
            const QJsonValue geometry = feature.value(QStringLiteral("geometry"));
            record.insert(QStringLiteral("geometryJson"),
                          geometry.isObject()
                              ? QString::fromUtf8(QJsonDocument(geometry.toObject()).toJson(QJsonDocument::Compact))
                              : QString());
            record.insert(QStringLiteral("geometryType"), geometry.isObject()
                              ? geometry.toObject().value(QStringLiteral("type")).toString()
                              : QString());
            record.insert(QStringLiteral("geometryAvailable"), geometry.isObject()
                              && !geometry.toObject().isEmpty());
            if (record.value(QStringLiteral("geometryAvailable")).toBool()) ++featuresWithGeometry;
            normalizedRecords.append(record);
        }

        // Publish a new immutable-by-convention result set only after schema/status checks.
        m_notamRecords = normalizedRecords;
        m_notamSnapshotId = QStringLiteral("FAA-NMS-%1-%2")
            .arg(m_environment, QUuid::createUuid().toString(QUuid::WithoutBraces));
        emit notamRecordsChanged();
        emit notamQueryFinished(true,
            QStringLiteral("FAA NMS query succeeded: %1 GeoJSON records normalized; %2 include geometry. Snapshot %3. Raw altitude limits are preserved with UNKNOWN datum; this snapshot is not yet connected to route validation.")
                .arg(normalizedRecords.size()).arg(featuresWithGeometry).arg(m_notamSnapshotId));
    });
}

void FaaNmsClient::requestPing(const QString &accessToken)
{
    QUrl url = baseUrl();
    url.setPath(QStringLiteral("/nmsapi/v1/ping"));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
    QNetworkReply *reply = m_network->get(request);
    QTimer *timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, reply, [reply]() {
        reply->setProperty("blueskyTimedOut", true);
        reply->abort();
    });
    timeout->start(15000);

    connect(reply, &QNetworkReply::finished, this, [this, reply, timeout]() {
        timeout->stop();
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError networkError = reply->error();
        reply->readAll();
        reply->deleteLater();

        m_requestInProgress = false;
        if (networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300) {
            const bool timedOut = reply->property("blueskyTimedOut").toBool();
            const QString state = timedOut ? QStringLiteral("TIMEOUT")
                : ((httpStatus == 401 || httpStatus == 403)
                ? QStringLiteral("AUTH_FAILED")
                : (httpStatus == 429 ? QStringLiteral("RATE_LIMITED")
                                     : (networkError == QNetworkReply::TimeoutError
                                            ? QStringLiteral("TIMEOUT") : QStringLiteral("ERROR"))));
            const QString message = QStringLiteral("FAA NMS ping failed (HTTP %1, network error %2).")
                                        .arg(httpStatus).arg(static_cast<int>(networkError));
            setStatus(state);
            setLastError(message);
            emit connectionTestFinished(false, message);
            return;
        }

        setStatus(QStringLiteral("CONNECTED"));
        setLastError({});
        emit connectionTestFinished(true, QStringLiteral("FAA NMS %1 token exchange and ping succeeded.")
                                           .arg(m_environment));
    });
}
