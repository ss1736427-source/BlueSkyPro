#include "FaaNmsClient.h"

#include "CredentialVault.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QTimer>

FaaNmsClient::FaaNmsClient(CredentialVault *vault, QObject *parent)
    : QObject(parent), m_vault(vault), m_network(new QNetworkAccessManager(this))
{
}

QString FaaNmsClient::status() const { return m_status; }
QString FaaNmsClient::lastError() const { return m_lastError; }
QString FaaNmsClient::environment() const { return m_environment; }

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
        const int expiresIn = object.value(QStringLiteral("expires_in")).toInt();
        if (parseError.error != QJsonParseError::NoError || !document.isObject()
            || accessToken.isEmpty() || expiresIn <= 0) {
            finishFailure(QStringLiteral("INVALID_RESPONSE"),
                          QStringLiteral("FAA NMS token response is malformed or missing required fields."));
            return;
        }
        requestPing(accessToken);
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
