#pragma once

#include <QObject>
#include <QProcess>
#include <QVariantMap>
#include <QVariantList>

#include "../../../04_SOFTWARE/PLANNING/mission_lifecycle_owner.hpp"

#include <optional>

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

    // The bridge owns the authoritative lifecycle store for the HMI runtime.
    // These APIs are C++ only: QML cannot fabricate lifecycle transitions.
    bool registerMissionLifecycle(const bluesky::planning::Mission& mission,
                                  std::vector<std::string>* blockers = nullptr);
    bluesky::planning::MissionLifecycleTransitionResult transitionMissionLifecycle(
        const std::string& mission_id,
        bluesky::planning::MissionLifecycleState target,
        const bluesky::planning::MissionLifecycleEvidence& evidence,
        const std::string& actor_id,
        const std::string& timestamp,
        const std::string& reason);
    // C++ runtime entry point: finalizes READY only from typed authoritative
    // service outputs, then publishes the owner snapshot to the HMI.
    bluesky::planning::MissionLifecycleTransitionResult finalizeMissionReadyFromAuthoritativeSources(
        const std::string& mission_id,
        const bluesky::planning::MissionLifecycleEvidence& evidence,
        const bluesky::planning::MissionLifecycleAuthoritativeSources& sources,
        const bluesky::planning::RegulatoryComplianceAssessment& regulatory_assessment,
        std::uint64_t now_epoch_s,
        const bluesky::planning::FlightProfile& selected_flight_profile,
        const bluesky::planning::VehicleCompilationInput& assigned_vehicle,
        const std::string& actor_id,
        const std::string& timestamp,
        const std::string& reason,
        const bluesky::planning::MissionLifecycleMt01QualityInputs* mt01_quality = nullptr,
        const QVariantList& pilotActions = {});
    std::optional<bluesky::planning::MissionLifecycleRecord>
    missionLifecycleSnapshot(const std::string& mission_id) const;
    bool publishOwnedLifecycleReadiness(
        const std::string& mission_id,
        const bluesky::planning::MissionLifecycleTransitionResult& transition,
        const QVariantList& pilotActions);
    bool publishOwnedLifecycleAssessment(
        const std::string& mission_id,
        const bluesky::planning::MissionLifecycleTransitionResult& assessment,
        const QVariantList& pilotActions);

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

    Q_INVOKABLE bool synchronizeDraftMissionLifecycle(const QString& missionModelJson);
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
    bluesky::planning::MissionLifecycleOwner m_lifecycleOwner;
};
