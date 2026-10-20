#pragma once

#include <QObject>
#include <QString>

class ManualGeometryBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString geometryJson READ geometryJson NOTIFY geometryChanged)
    Q_PROPERTY(bool compositionComplete READ compositionComplete NOTIFY compositionCompleteChanged)

public:
    explicit ManualGeometryBridge(QObject *parent = nullptr)
        : QObject(parent)
    {
        // Give Qt WebChannel a stable QObject name instead of relying on
        // QQmlContext::nameForObject() for a C++ context property.
        setObjectName(QStringLiteral("manualGeometryBridge"));
    }

    QString geometryJson() const { return m_geometryJson; }
    bool compositionComplete() const { return m_compositionComplete; }

public slots:
    void publishGeometry(const QString &json, bool complete);

signals:
    void geometryChanged();
    void compositionCompleteChanged();
    void bridgeError(const QString &code);

private:
    QString m_geometryJson = QStringLiteral("{\"objects\":[]}");
    bool m_compositionComplete = false;
};
