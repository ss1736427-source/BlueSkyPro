#pragma once

#include <QObject>
#include <QVariantMap>

class PlanningBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged)

public:
    explicit PlanningBridge(QObject *parent = nullptr);

    QVariantMap result() const;

    Q_INVOKABLE void clear();

public slots:
    void publishResult(const QVariantMap &result);

signals:
    void resultChanged();

private:
    QVariantMap m_result;
};
