#pragma once

#include <QObject>
#include <QProcess>
#include <QVariantMap>
#include <QVariantList>

#include "../../../04_SOFTWARE/PLANNING/mission_lifecycle.hpp"

class PlanningBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QVariantMap readinessSnapshot READ readinessSnapshot NOTIFY readinessSnapshotChanged)

public:
    explicit PlanningBridge(QObject *parent = nullptr);

    QVariantMap result() const;
    bool running() const;
    QVariantMap readinessSnapshot() const;

    // C++ lifecycle owners only; deliberately not Q_INVOKABLE.
    bool publishLifecycleReadiness(
        const bluesky::planning::MissionLifecycleRecord &record,
        const bluesky::planning::MissionLifecycleTransitionResult &transition,
        const QVariantList &pilotActions);
    // Publishes a fail-closed assessment while the mission is still VALIDATING,
    // so pilot-owned actions are visible before READY can be granted.
    bool publishLifecycleAssessment(
        const bluesky::planning::MissionLifecycleRecord &record,
        const bluesky::planning::MissionLifecycleTransitionResult &assessment,
        const QVariantList &pilotActions);

    Q_INVOKABLE bool startProcess(const QString &program, const QStringList &arguments = {});
    Q_INVOKABLE void stopProcess();
    Q_INVOKABLE bool sendRequest(const QString &json);
    Q_INVOKABLE bool publishJson(const QString &json);
    Q_INVOKABLE void clear();

signals:
    void resultChanged();
    void runningChanged();
    void readinessSnapshotChanged();
    void bridgeError(const QString &code);

private:
    // Internal publication only; QML must not be able to bypass publishJson's
    // fail-closed lifecycle gate by injecting an arbitrary QVariantMap.
    void publishResult(const QVariantMap &result);
    void consumeStdout();
    void invalidateLifecycleReadiness(const QString &reason);

    QProcess m_process;
    QByteArray m_stdoutBuffer;
    QVariantMap m_result;
    QVariantMap m_readinessSnapshot;
};
