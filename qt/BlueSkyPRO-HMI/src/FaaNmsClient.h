#pragma once

#include <QObject>
#include <QUrl>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QHash>
#include <QList>

class CredentialVault;
class QNetworkAccessManager;
class QNetworkReply;

class FaaNmsClient final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    Q_PROPERTY(QVariantList notamRecords READ notamRecords NOTIFY notamRecordsChanged)
    Q_PROPERTY(QString notamSnapshotId READ notamSnapshotId NOTIFY notamRecordsChanged)
    Q_PROPERTY(qint64 notamSnapshotEpochS READ notamSnapshotEpochS NOTIFY notamRecordsChanged)
    Q_PROPERTY(double notamQueryLatitude READ notamQueryLatitude NOTIFY notamRecordsChanged)
    Q_PROPERTY(double notamQueryLongitude READ notamQueryLongitude NOTIFY notamRecordsChanged)
    Q_PROPERTY(double notamQueryRadiusM READ notamQueryRadiusM NOTIFY notamRecordsChanged)

public:
    explicit FaaNmsClient(CredentialVault *vault, QObject *parent = nullptr);
    QString status() const;
    QString lastError() const;
    QString environment() const;
    QVariantList notamRecords() const;
    QString notamSnapshotId() const;
    qint64 notamSnapshotEpochS() const;
    double notamQueryLatitude() const;
    double notamQueryLongitude() const;
    double notamQueryRadiusM() const;
    void setEnvironment(const QString &environment);

    Q_INVOKABLE void testConnection();
    Q_INVOKABLE void queryNotams(double latitude, double longitude, double radiusNm);

signals:
    void statusChanged();
    void lastErrorChanged();
    void environmentChanged();
    void notamRecordsChanged();
    void connectionTestFinished(bool success, const QString &message);
    void notamQueryFinished(bool success, const QString &message);

private:
    void setStatus(const QString &status);
    void setLastError(const QString &error);
    void requestPing(const QString &accessToken);
    void requestNotams(const QString &accessToken, double latitude, double longitude, double radiusNm);
    void finishFailure(const QString &status, const QString &message);
    QUrl baseUrl() const;

    CredentialVault *m_vault = nullptr;
    QNetworkAccessManager *m_network = nullptr;
    QString m_status = QStringLiteral("NOT_CONFIGURED");
    QString m_lastError;
    QString m_environment = QStringLiteral("STAGING");
    bool m_requestInProgress = false;
    bool m_queryInProgress = false;
    QVariantList m_notamRecords;
    // Bounded content-addressed cache: unchanged records are not reparsed on viewport refresh.
    QHash<QString, QVariantMap> m_interpretationCache;
    QList<QString> m_interpretationCacheOrder;
    QString m_notamSnapshotId;
    qint64 m_notamSnapshotEpochS = 0;
    double m_notamQueryLatitude = 0.0;
    double m_notamQueryLongitude = 0.0;
    double m_notamQueryRadiusM = 0.0;
};
