#pragma once

#include <QDateTime>
#include <QHash>
#include <QDirIterator>
#include <QNetworkAccessManager>
#include <QQueue>
#include <QSet>
#include <QTimer>
#include <QObject>

#include <memory>

class QNetworkReply;

class TileCacheManager final : public QObject
{
    Q_OBJECT

public:
    explicit TileCacheManager(QObject *parent = nullptr);

    Q_INVOKABLE void beginViewUpdate();
    Q_INVOKABLE QString requestTile(const QString &key, const QString &url);

signals:
    void tileReady(const QString &key, const QString &fileUrl);
    void tileFailed(const QString &key);

private:
    struct Request {
        QString key;
        QString url;
        int retryCount = 0;
        quint64 viewGeneration = 0;
    };

    void enqueue(const Request &request);
    void pump();
    void handleFinished(QNetworkReply *reply);

    QString cacheRootPath() const;
    QString cachePath(const QString &key) const;
    QDateTime cacheExpiry(const QString &tilePath) const;
    void removeCacheEntry(const QString &tilePath) const;
    void cleanupExpiredCacheBatch();
    static bool isTransientFailure(QNetworkReply *reply);

    QNetworkAccessManager m_network;
    QQueue<Request> m_queue;
    QSet<QString> m_pending;
    QHash<QNetworkReply *, Request> m_activeRequests;
    QTimer m_pumpTimer;
    QTimer m_cleanupTimer;
    std::unique_ptr<QDirIterator> m_cleanupIterator;
    int m_activeCount = 0;
    quint64 m_viewGeneration = 0;
    qint64 m_lastStartMs = 0;
};
