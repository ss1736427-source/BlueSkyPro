#pragma once

#include <QCryptographicHash>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QStringList>

class QFile;
class QNetworkReply;

class WeatherDataService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)
    Q_PROPERTY(QString lastDownloadedFile READ lastDownloadedFile NOTIFY forecastReady)

public:
    explicit WeatherDataService(QObject *parent = nullptr);

    QString state() const;
    QString lastError() const;
    QString cacheDirectory() const;
    QString lastDownloadedFile() const;

    // Downloads a bounded NOAA GFS 0.25-degree GRIB2 wind subset asynchronously.
    // Supported direct AGL levels are confirmed by the current NOMADS filter:
    // 10, 20, 30, 40, 50, 80, 100 and 1000 metres.
    Q_INVOKABLE bool requestWindSubset(const QString &dateUtc,
                                       const QString &cycleUtc,
                                       int forecastHour,
                                       double west,
                                       double south,
                                       double east,
                                       double north,
                                       const QStringList &levelsAglM);

    Q_INVOKABLE void cancelPending();

    static QUrl buildRequestUrl(const QString &dateUtc,
                                const QString &cycleUtc,
                                int forecastHour,
                                double west,
                                double south,
                                double east,
                                double north,
                                const QStringList &levelsAglM,
                                QString *error = nullptr);
    static bool isValidGrib2File(const QString &path, QString *error = nullptr);

signals:
    void stateChanged();
    void lastErrorChanged();
    void progressChanged(qint64 receivedBytes, qint64 maximumBytes);
    void forecastReady(const QString &filePath, const QVariantMap &manifest);
    void downloadFailed(const QString &reason);

private:
    void setState(const QString &value);
    void setError(const QString &value);
    void consumeReplyData();
    void finishReply();
    void failDownload(const QString &reason);
    bool writeManifest(const QJsonObject &manifest, QString *error = nullptr);

    static constexpr qint64 kMaximumDownloadBytes = 256LL * 1024LL * 1024LL;

    QNetworkAccessManager m_network;
    QPointer<QNetworkReply> m_reply;
    QFile *m_partialFile{nullptr};
    QCryptographicHash m_hash{QCryptographicHash::Sha256};
    QString m_state{QStringLiteral("IDLE")};
    QString m_lastError;
    QString m_cacheDirectory;
    QString m_lastDownloadedFile;
    QString m_partialPath;
    QString m_finalPath;
    QString m_manifestPath;
    QJsonObject m_manifest;
    qint64 m_receivedBytes{0};
};
