#pragma once

#include <QObject>
#include <QUrl>
#include <QString>

class CredentialVault;
class QNetworkAccessManager;
class QNetworkReply;

class FaaNmsClient final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString environment READ environment WRITE setEnvironment NOTIFY environmentChanged)

public:
    explicit FaaNmsClient(CredentialVault *vault, QObject *parent = nullptr);
    QString status() const;
    QString lastError() const;
    QString environment() const;
    void setEnvironment(const QString &environment);

    Q_INVOKABLE void testConnection();

signals:
    void statusChanged();
    void lastErrorChanged();
    void environmentChanged();
    void connectionTestFinished(bool success, const QString &message);

private:
    void setStatus(const QString &status);
    void setLastError(const QString &error);
    void requestPing(const QString &accessToken);
    void finishFailure(const QString &status, const QString &message);
    QUrl baseUrl() const;

    CredentialVault *m_vault = nullptr;
    QNetworkAccessManager *m_network = nullptr;
    QString m_status = QStringLiteral("NOT_CONFIGURED");
    QString m_lastError;
    QString m_environment = QStringLiteral("STAGING");
    bool m_requestInProgress = false;
};
