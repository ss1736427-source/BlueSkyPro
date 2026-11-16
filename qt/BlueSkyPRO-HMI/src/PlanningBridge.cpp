#include "PlanningBridge.h"
#include "../../../04_SOFTWARE/PLANNING/mission_lifecycle_validation_coordinator.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <array>
#include <cmath>

namespace
{
constexpr auto kSchemaVersion = "1.0";
constexpr auto kMessageType = "planning.result";

const char *lifecycleStateText(bluesky::planning::MissionLifecycleState state)
{
    using bluesky::planning::MissionLifecycleState;
    switch (state) {
    case MissionLifecycleState::Draft: return "DRAFT";
    case MissionLifecycleState::Calculating: return "CALCULATING";
    case MissionLifecycleState::Validating: return "VALIDATING";
    case MissionLifecycleState::Ready: return "READY";
    case MissionLifecycleState::Submitted: return "SUBMITTED";
    case MissionLifecycleState::Authorized: return "AUTHORIZED";
    case MissionLifecycleState::Released: return "RELEASED";
    case MissionLifecycleState::Executing: return "EXECUTING";
    case MissionLifecycleState::Paused: return "PAUSED";
    case MissionLifecycleState::Replanning: return "REPLANNING";
    case MissionLifecycleState::Completed: return "COMPLETED";
    case MissionLifecycleState::Aborted: return "ABORTED";
    }
    return "UNKNOWN";
}
}

PlanningBridge::PlanningBridge(QObject *parent)
    : QObject(parent)
{
    invalidateLifecycleReadiness(QStringLiteral("CPP_MISSION_FINALIZATION_REQUIRED"));
    connect(&m_process, &QProcess::readyReadStandardOutput,
            this, &PlanningBridge::consumeStdout);

    connect(&m_process, &QProcess::errorOccurred, this,
            [this](QProcess::ProcessError) {
                emit bridgeError(QStringLiteral("PLANNING_PROCESS_ERROR"));
            });

    connect(&m_process, &QProcess::started, this, [this]() {
        emit runningChanged();
    });

    connect(&m_process, &QProcess::finished, this,
            [this](int, QProcess::ExitStatus) {
                emit runningChanged();
                consumeStdout();
            });
}

QVariantMap PlanningBridge::result() const
{
    return m_result;
}

bool PlanningBridge::running() const
{
    return m_process.state() != QProcess::NotRunning;
}

QVariantMap PlanningBridge::readinessSnapshot() const
{
    return m_readinessSnapshot;
}

bool PlanningBridge::synchronizeDraftMissionLifecycle(const QString& missionModelJson)
{
    const QJsonDocument document = QJsonDocument::fromJson(missionModelJson.toUtf8());
    if (!document.isObject()) {
        emit bridgeError(QStringLiteral("MISSION_MODEL_JSON_INVALID"));
        return false;
    }

    const QJsonObject root = document.object();
    const QString missionId = root.value(QStringLiteral("missionId")).toString().trimmed();
    const QJsonValue versionValue = root.value(QStringLiteral("missionVersion"));
    const double versionNumber = versionValue.isString()
        ? versionValue.toString().toDouble() : versionValue.toDouble(-1.0);
    const QString missionVersion = versionValue.isString()
        ? versionValue.toString().trimmed()
        : (versionValue.isDouble() && versionNumber > 0.0
            && std::floor(versionNumber) == versionNumber
            ? QString::number(static_cast<qlonglong>(versionNumber)) : QString());
    const QJsonObject lifecycleJson = root.value(QStringLiteral("lifecycle")).toObject();
    const QJsonValue templatesValue = root.value(QStringLiteral("selectedTemplateIds"));
    const QString missionMode = root.value(QStringLiteral("missionMode")).toString();
    const QJsonValue lifecycleVersionValue = lifecycleJson.value(QStringLiteral("missionVersion"));
    const QString lifecycleVersion = lifecycleVersionValue.isString()
        ? lifecycleVersionValue.toString().trimmed()
        : (lifecycleVersionValue.isDouble()
            && lifecycleVersionValue.toDouble(-1.0) == versionNumber
            ? QString::number(static_cast<qlonglong>(versionNumber)) : QString());
    const QJsonObject executionContext = root.value(QStringLiteral("executionContext")).toObject();
    const QString mode = executionContext.value(QStringLiteral("mode")).toString();
    const bool virtualMode = mode == QLatin1String("VIRTUAL");
    const bool physicalMode = mode == QLatin1String("PHYSICAL");

    if (missionId.isEmpty() || missionVersion.isEmpty()
        || root.value(QStringLiteral("status")).toString() != QLatin1String("DRAFT")
        || (missionMode != QLatin1String("AUTO") && missionMode != QLatin1String("MANUAL"))
        || !templatesValue.isArray() || templatesValue.toArray().isEmpty()
        || lifecycleJson.value(QStringLiteral("state")).toString() != QLatin1String("DRAFT")
        || lifecycleJson.value(QStringLiteral("missionId")).toString() != missionId
        || lifecycleVersion != missionVersion
        || !lifecycleJson.value(QStringLiteral("history")).toArray().isEmpty()
        || lifecycleJson.value(QStringLiteral("nextEventSequence")).toInt() != 1
        || (!virtualMode && !physicalMode)
        || executionContext.value(QStringLiteral("vehicleSourceProvenanceRequired")).toString()
            != (virtualMode ? QLatin1String("SIL_FIXTURE") : QLatin1String("PHYSICAL_LIVE"))
        || executionContext.value(QStringLiteral("releaseTarget")).toString()
            != (virtualMode ? QLatin1String("SIMULATOR_ONLY") : QLatin1String("PHYSICAL_UAV"))
        || executionContext.value(QStringLiteral("authorizationPath")).toString()
            != (virtualMode ? QLatin1String("NOT_APPLICABLE") : QLatin1String("EXTERNAL_REGULATORY"))
        || executionContext.value(QStringLiteral("esorvdSubmissionAllowed")).toBool() == virtualMode) {
        emit bridgeError(QStringLiteral("MISSION_DRAFT_EXECUTION_CONTEXT_INVALID"));
        return false;
    }

    bluesky::planning::Mission mission;
    mission.identity.mission_id = missionId.toStdString();
    mission.identity.mission_version = missionVersion.toStdString();
    mission.lifecycle.mission_id = mission.identity.mission_id;
    mission.lifecycle.mission_version = mission.identity.mission_version;
    mission.lifecycle.execution_mode = virtualMode
        ? bluesky::planning::MissionExecutionMode::Virtual
        : bluesky::planning::MissionExecutionMode::Physical;
    mission.mission_mode = missionMode.toStdString();

    const QJsonArray templates = templatesValue.toArray();
    for (const QJsonValue& value : templates) {
        if (!value.isString() || value.toString().trimmed().isEmpty()) {
            emit bridgeError(QStringLiteral("MISSION_TEMPLATE_ID_INVALID"));
            return false;
        }
        mission.selected_template_ids.push_back(value.toString().toStdString());
    }

    std::vector<std::string> blockers;
    if (!m_lifecycleOwner.synchronizeDraftMission(mission, &blockers)) {
        const QString code = blockers.empty()
            ? QStringLiteral("MISSION_DRAFT_REGISTRATION_REJECTED")
            : QString::fromStdString(blockers.front());
        emit bridgeError(code);
        return false;
    }
    return true;
}

bool PlanningBridge::registerMissionLifecycle(
    const bluesky::planning::Mission& mission,
    std::vector<std::string>* blockers)
{
    return m_lifecycleOwner.registerMission(mission, blockers);
}

bluesky::planning::MissionLifecycleTransitionResult
PlanningBridge::transitionMissionLifecycle(
    const std::string& mission_id,
    bluesky::planning::MissionLifecycleState target,
    const bluesky::planning::MissionLifecycleEvidence& evidence,
    const std::string& actor_id,
    const std::string& timestamp,
    const std::string& reason)
{
    return m_lifecycleOwner.transition(
        mission_id, target, evidence, actor_id, timestamp, reason);
}

bluesky::planning::MissionLifecycleTransitionResult
PlanningBridge::finalizeMissionReadyFromAuthoritativeSources(
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
    const bluesky::planning::MissionLifecycleMt01QualityInputs* mt01_quality,
    const QVariantList& pilotActions)
{
    auto transition = m_lifecycleOwner.finalizeReadyFromAuthoritativeSources(
        mission_id, evidence, sources, regulatory_assessment, now_epoch_s,
        selected_flight_profile, assigned_vehicle, actor_id, timestamp,
        reason, mt01_quality);
    if (transition.accepted) {
        if (!publishOwnedLifecycleReadiness(mission_id, transition, pilotActions)) {
            transition.accepted = false;
            transition.blockers.emplace_back("LIFECYCLE_READINESS_PUBLICATION_REJECTED");
        }
    } else {
        // A failed assessment can expose pilot-owned actions, but it never
        // grants READY. System-completed checks stay out of this list.
        publishOwnedLifecycleAssessment(mission_id, transition, pilotActions);
    }
    return transition;
}

std::optional<bluesky::planning::MissionLifecycleRecord>
PlanningBridge::missionLifecycleSnapshot(const std::string& mission_id) const
{
    return m_lifecycleOwner.snapshot(mission_id);
}

bool PlanningBridge::publishOwnedLifecycleReadiness(
    const std::string& mission_id,
    const bluesky::planning::MissionLifecycleTransitionResult& transition,
    const QVariantList& pilotActions)
{
    const auto record = m_lifecycleOwner.snapshot(mission_id);
    if (!record) {
        invalidateLifecycleReadiness(QStringLiteral("MISSION_NOT_REGISTERED"));
        return false;
    }
    return publishLifecycleReadiness(*record, transition, pilotActions);
}

bool PlanningBridge::publishOwnedLifecycleAssessment(
    const std::string& mission_id,
    const bluesky::planning::MissionLifecycleTransitionResult& assessment,
    const QVariantList& pilotActions)
{
    const auto record = m_lifecycleOwner.snapshot(mission_id);
    if (!record) {
        invalidateLifecycleReadiness(QStringLiteral("MISSION_NOT_REGISTERED"));
        return false;
    }
    return publishLifecycleAssessment(*record, assessment, pilotActions);
}

void PlanningBridge::invalidateLifecycleReadiness(const QString &reason)
{
    QVariantMap snapshot{
        {QStringLiteral("status"), QStringLiteral("BLOCKED")},
        {QStringLiteral("executionMode"), QString()},
        {QStringLiteral("lifecycleState"), QStringLiteral("BLOCKED")},
        {QStringLiteral("ready"), false},
        {QStringLiteral("released"), false},
        {QStringLiteral("missionId"), QString()},
        {QStringLiteral("missionVersion"), QString()},
        {QStringLiteral("vehicleId"), QString()},
        {QStringLiteral("vehicleSourceProvenance"), QString()},
        {QStringLiteral("referenceModelId"), QString()},
        {QStringLiteral("vehicleConfigurationId"), QString()},
        {QStringLiteral("vehicleConfigurationRevision"), QString()},
        {QStringLiteral("performanceProfileId"), QString()},
        {QStringLiteral("performanceProfileRevision"), QString()},
        {QStringLiteral("routeId"), QString()},
        {QStringLiteral("routeVersion"), QString()},
        {QStringLiteral("calculationInputVersion"), QString()},
        {QStringLiteral("blockers"), QVariantList{reason}},
        {QStringLiteral("pilotActions"), QVariantList{}}
    };
    if (m_readinessSnapshot == snapshot)
        return;
    m_readinessSnapshot = snapshot;
    emit readinessSnapshotChanged();
}

bool PlanningBridge::publishLifecycleReadiness(
    const bluesky::planning::MissionLifecycleRecord &record,
    const bluesky::planning::MissionLifecycleTransitionResult &transition,
    const QVariantList &pilotActions)
{
    using namespace bluesky::planning;
    const auto invalidate = [this]() {
        invalidateLifecycleReadiness(QStringLiteral("CPP_LIFECYCLE_SNAPSHOT_INVALID"));
        return false;
    };
    if (!transition.accepted || !transition.blockers.empty() || transition.state != record.state
        || record.mission_id.empty() || record.mission_version.empty()
        || record.vehicle_id.empty()
        || (record.execution_mode == MissionExecutionMode::Physical
            && record.vehicle_source_provenance != "PHYSICAL_LIVE")
        || (record.execution_mode == MissionExecutionMode::Virtual
            && record.vehicle_source_provenance != "SIL_FIXTURE")
        || record.reference_model_id.empty() || record.vehicle_configuration_id.empty()
        || record.vehicle_configuration_revision.empty()
        || record.performance_profile_id.empty() || record.performance_profile_revision.empty()
        || record.current_route_id.empty() || record.current_route_version.empty()
        || record.current_calculation_input_version.empty() || record.history.empty())
        return invalidate();

    std::size_t postReadyTransitions = 0;
    if (record.execution_mode == MissionExecutionMode::Virtual) {
        if (record.state == MissionLifecycleState::Released)
            postReadyTransitions = 1;
        else if (record.state != MissionLifecycleState::Ready)
            return invalidate();
    } else {
        switch (record.state) {
        case MissionLifecycleState::Ready: postReadyTransitions = 0; break;
        case MissionLifecycleState::Submitted: postReadyTransitions = 1; break;
        case MissionLifecycleState::Authorized: postReadyTransitions = 2; break;
        case MissionLifecycleState::Released: postReadyTransitions = 3; break;
        default: return invalidate();
        }
    }

    std::array<MissionLifecycleState, 4> expectedFrom{
        MissionLifecycleState::Validating, MissionLifecycleState::Ready,
        MissionLifecycleState::Submitted, MissionLifecycleState::Authorized};
    std::array<MissionLifecycleState, 4> expectedTo{
        MissionLifecycleState::Ready, MissionLifecycleState::Submitted,
        MissionLifecycleState::Authorized, MissionLifecycleState::Released};
    if (record.execution_mode == MissionExecutionMode::Virtual) {
        expectedFrom[1] = MissionLifecycleState::Ready;
        expectedTo[1] = MissionLifecycleState::Released;
    }
    const auto &history = record.history;
    std::uint64_t previousSequence = 0;
    std::size_t readyIndex = history.size();
    for (std::size_t i = 0; i < history.size(); ++i) {
        const auto &event = history[i];
        if (event.sequence == 0 || event.sequence <= previousSequence
            || event.actor_id.empty() || event.timestamp.empty()
            || event.execution_mode != record.execution_mode
            || event.vehicle_id != record.vehicle_id
            || event.vehicle_source_provenance != record.vehicle_source_provenance
            || event.reference_model_id != record.reference_model_id
            || event.vehicle_configuration_id != record.vehicle_configuration_id
            || event.vehicle_configuration_revision != record.vehicle_configuration_revision
            || event.performance_profile_id != record.performance_profile_id
            || event.performance_profile_revision != record.performance_profile_revision)
            return invalidate();
        previousSequence = event.sequence;
        if (event.mission_id == record.mission_id
            && event.mission_version == record.mission_version
            && event.from == MissionLifecycleState::Validating
            && event.to == MissionLifecycleState::Ready
            && event.route_id == record.current_route_id
            && event.route_version == record.current_route_version
            && event.evidence_reference == record.current_calculation_input_version)
            readyIndex = i;
    }
    if (readyIndex == history.size()
        || history.size() != readyIndex + 1 + postReadyTransitions
        || record.next_event_sequence != history.back().sequence + 1)
        return invalidate();

    for (std::size_t stage = 0; stage <= postReadyTransitions; ++stage) {
        const auto &event = history[readyIndex + stage];
        if (event.mission_id != record.mission_id
            || event.mission_version != record.mission_version
            || event.from != expectedFrom[stage] || event.to != expectedTo[stage]
            || event.route_id != record.current_route_id
            || event.route_version != record.current_route_version
            || event.evidence_reference != record.current_calculation_input_version)
            return invalidate();
        if (stage > 0 && event.sequence != history[readyIndex + stage - 1].sequence + 1)
            return invalidate();
    }
    if (history.back().to != record.state)
        return invalidate();

    QVariantList filteredPilotActions;
    for (const QVariant &value : pilotActions) {
        const QVariantMap action = value.toMap();
        if (!action.value(QStringLiteral("requiresHuman")).toBool())
            continue;
        const QString actionId = action.value(QStringLiteral("actionId")).toString().trimmed();
        const QString label = action.value(QStringLiteral("label")).toString().trimmed();
        if (actionId.isEmpty() || label.isEmpty())
            continue;
        filteredPilotActions.push_back(action);
    }

    const QString state = QString::fromLatin1(lifecycleStateText(record.state));
    QVariantMap snapshot{
        {QStringLiteral("status"), state},
        {QStringLiteral("lifecycleState"), state},
        {QStringLiteral("executionMode"), QString::fromLatin1(toString(record.execution_mode))},
        {QStringLiteral("ready"), true},
        {QStringLiteral("released"), record.state == MissionLifecycleState::Released},
        {QStringLiteral("missionId"), QString::fromStdString(record.mission_id)},
        {QStringLiteral("missionVersion"), QString::fromStdString(record.mission_version)},
        {QStringLiteral("vehicleId"), QString::fromStdString(record.vehicle_id)},
        {QStringLiteral("vehicleSourceProvenance"), QString::fromStdString(record.vehicle_source_provenance)},
        {QStringLiteral("referenceModelId"), QString::fromStdString(record.reference_model_id)},
        {QStringLiteral("vehicleConfigurationId"), QString::fromStdString(record.vehicle_configuration_id)},
        {QStringLiteral("vehicleConfigurationRevision"), QString::fromStdString(record.vehicle_configuration_revision)},
        {QStringLiteral("performanceProfileId"), QString::fromStdString(record.performance_profile_id)},
        {QStringLiteral("performanceProfileRevision"), QString::fromStdString(record.performance_profile_revision)},
        {QStringLiteral("routeId"), QString::fromStdString(record.current_route_id)},
        {QStringLiteral("routeVersion"), QString::fromStdString(record.current_route_version)},
        {QStringLiteral("calculationInputVersion"), QString::fromStdString(record.current_calculation_input_version)},
        {QStringLiteral("blockers"), QVariantList{}},
        {QStringLiteral("pilotActions"), filteredPilotActions}
    };
    if (m_readinessSnapshot != snapshot) {
        m_readinessSnapshot = snapshot;
        emit readinessSnapshotChanged();
    }
    return true;
}

bool PlanningBridge::publishLifecycleAssessment(
    const bluesky::planning::MissionLifecycleRecord &record,
    const bluesky::planning::MissionLifecycleTransitionResult &assessment,
    const QVariantList &pilotActions)
{
    using namespace bluesky::planning;
    const auto invalidate = [this]() {
        invalidateLifecycleReadiness(QStringLiteral("CPP_LIFECYCLE_ASSESSMENT_INVALID"));
        return false;
    };

    // This path is deliberately separate from publishLifecycleReadiness:
    // it can publish blockers and pilot actions, but can never grant READY.
    if (record.state != MissionLifecycleState::Validating
        || assessment.state != record.state
        || assessment.accepted
        || record.mission_id.empty() || record.mission_version.empty()
        || record.vehicle_id.empty()
        || (record.vehicle_source_provenance != "PHYSICAL_LIVE"
            && record.vehicle_source_provenance != "SIL_FIXTURE")
        || record.current_route_id.empty() || record.current_route_version.empty()
        || record.current_calculation_input_version.empty() || record.history.empty())
        return invalidate();

    const auto &lastEvent = record.history.back();
    if (lastEvent.sequence == 0
        || record.next_event_sequence != lastEvent.sequence + 1
        || lastEvent.execution_mode != record.execution_mode
        || lastEvent.mission_id != record.mission_id
        || lastEvent.mission_version != record.mission_version
        || lastEvent.from != MissionLifecycleState::Calculating
        || lastEvent.to != MissionLifecycleState::Validating
        || lastEvent.route_id != record.current_route_id
        || lastEvent.route_version != record.current_route_version
        || lastEvent.evidence_reference != record.current_calculation_input_version
        || lastEvent.actor_id.empty() || lastEvent.timestamp.empty()
        || lastEvent.vehicle_id != record.vehicle_id
        || lastEvent.vehicle_source_provenance != record.vehicle_source_provenance
        || lastEvent.reference_model_id != record.reference_model_id
        || lastEvent.vehicle_configuration_id != record.vehicle_configuration_id
        || lastEvent.vehicle_configuration_revision != record.vehicle_configuration_revision
        || lastEvent.performance_profile_id != record.performance_profile_id
        || lastEvent.performance_profile_revision != record.performance_profile_revision)
        return invalidate();

    QVariantList filteredPilotActions;
    for (const QVariant &value : pilotActions) {
        const QVariantMap action = value.toMap();
        if (!action.value(QStringLiteral("requiresHuman")).toBool())
            continue;
        const QString actionId = action.value(QStringLiteral("actionId")).toString().trimmed();
        const QString label = action.value(QStringLiteral("label")).toString().trimmed();
        if (actionId.isEmpty() || label.isEmpty())
            continue;
        filteredPilotActions.push_back(action);
    }

    QVariantList blockers;
    for (const std::string &blocker : assessment.blockers)
        blockers.push_back(QString::fromStdString(blocker));
    if (blockers.isEmpty())
        blockers.push_back(QStringLiteral("MISSION_READY_FINALIZATION_NOT_ACCEPTED"));

    QVariantMap snapshot{
        {QStringLiteral("status"), QStringLiteral("VALIDATING")},
        {QStringLiteral("lifecycleState"), QStringLiteral("VALIDATING")},
        {QStringLiteral("executionMode"), QString::fromLatin1(toString(record.execution_mode))},
        {QStringLiteral("ready"), false},
        {QStringLiteral("released"), false},
        {QStringLiteral("missionId"), QString::fromStdString(record.mission_id)},
        {QStringLiteral("missionVersion"), QString::fromStdString(record.mission_version)},
        {QStringLiteral("vehicleId"), QString::fromStdString(record.vehicle_id)},
        {QStringLiteral("vehicleSourceProvenance"), QString::fromStdString(record.vehicle_source_provenance)},
        {QStringLiteral("referenceModelId"), QString::fromStdString(record.reference_model_id)},
        {QStringLiteral("vehicleConfigurationId"), QString::fromStdString(record.vehicle_configuration_id)},
        {QStringLiteral("vehicleConfigurationRevision"), QString::fromStdString(record.vehicle_configuration_revision)},
        {QStringLiteral("performanceProfileId"), QString::fromStdString(record.performance_profile_id)},
        {QStringLiteral("performanceProfileRevision"), QString::fromStdString(record.performance_profile_revision)},
        {QStringLiteral("routeId"), QString::fromStdString(record.current_route_id)},
        {QStringLiteral("routeVersion"), QString::fromStdString(record.current_route_version)},
        {QStringLiteral("calculationInputVersion"), QString::fromStdString(record.current_calculation_input_version)},
        {QStringLiteral("blockers"), blockers},
        {QStringLiteral("pilotActions"), filteredPilotActions}
    };
    if (m_readinessSnapshot != snapshot) {
        m_readinessSnapshot = snapshot;
        emit readinessSnapshotChanged();
    }
    return true;
}

bool PlanningBridge::startProcess(const QString &program, const QStringList &arguments)
{
    if (program.isEmpty()) {
        emit bridgeError(QStringLiteral("PLANNING_PROCESS_PROGRAM_REQUIRED"));
        return false;
    }

    if (running()) {
        emit bridgeError(QStringLiteral("PLANNING_PROCESS_ALREADY_RUNNING"));
        return false;
    }

    m_stdoutBuffer.clear();
    m_process.start(program, arguments);
    if (!m_process.waitForStarted(3000)) {
        emit bridgeError(QStringLiteral("PLANNING_PROCESS_START_FAILED"));
        return false;
    }

    return true;
}

void PlanningBridge::stopProcess()
{
    if (!running())
        return;

    m_process.terminate();
    if (!m_process.waitForFinished(1000))
        m_process.kill();
}

bool PlanningBridge::sendRequest(const QString &json)
{
    if (!running()) {
        emit bridgeError(QStringLiteral("PLANNING_PROCESS_NOT_RUNNING"));
        return false;
    }

    if (json.trimmed().isEmpty()) {
        emit bridgeError(QStringLiteral("EMPTY_PLANNING_REQUEST"));
        return false;
    }

    const QByteArray payload = json.toUtf8();
    if (!payload.endsWith('\n'))
        m_process.write(payload + '\n');
    else
        m_process.write(payload);

    return m_process.waitForBytesWritten(1000);
}

bool PlanningBridge::publishJson(const QString &json)
{
    const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8());
    if (!document.isObject()) {
        emit bridgeError(QStringLiteral("INVALID_JSON"));
        return false;
    }

    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("schemaVersion")).toString() != QLatin1String(kSchemaVersion)) {
        emit bridgeError(QStringLiteral("UNSUPPORTED_SCHEMA_VERSION"));
        return false;
    }

    if (object.value(QStringLiteral("messageType")).toString() != QLatin1String(kMessageType)) {
        emit bridgeError(QStringLiteral("UNSUPPORTED_MESSAGE_TYPE"));
        return false;
    }

    if (object.value(QStringLiteral("missionId")).toString().isEmpty()
        || object.value(QStringLiteral("resultId")).toString().isEmpty()) {
        emit bridgeError(QStringLiteral("MISSION_AND_RESULT_IDS_REQUIRED"));
        return false;
    }

    const QJsonObject verification = object.value(QStringLiteral("verification")).toObject();
    const QString releaseStatus = verification.value(QStringLiteral("releaseStatus")).toString();
    const QString finalGateStatus = verification.value(QStringLiteral("finalGateStatus")).toString();

    if ((releaseStatus != QLatin1String("RELEASE_ELIGIBLE")
         && releaseStatus != QLatin1String("BLOCKED"))
        || (finalGateStatus != QLatin1String("PASS")
            && finalGateStatus != QLatin1String("FAIL"))) {
        emit bridgeError(QStringLiteral("INVALID_VERIFICATION_STATUS"));
        return false;
    }

    const bool verified = verification.value(QStringLiteral("verified")).toBool();
    if (verified != (releaseStatus == QLatin1String("RELEASE_ELIGIBLE"))) {
        emit bridgeError(QStringLiteral("VERIFICATION_STATUS_MISMATCH"));
        return false;
    }

    // Python Planning Core may report its own planning gate, but it does not
    // execute the C++ MissionFinalizationRuntime or append a READY lifecycle
    // event. Therefore its positive release claim is never authoritative here.
    // Preserve the calculated route/candidate payload for map review, while
    // keeping the mission explicitly blocked until the C++ lifecycle runtime
    // supplies final-integrity and all other required validation attestations.
    QJsonObject normalized = object;
    QJsonObject normalizedVerification = verification;
    const bool claimsReady = releaseStatus == QLatin1String("RELEASE_ELIGIBLE")
        || finalGateStatus == QLatin1String("PASS") || verified;
    normalizedVerification.insert(QStringLiteral("releaseStatus"), QStringLiteral("BLOCKED"));
    normalizedVerification.insert(QStringLiteral("finalGateStatus"), QStringLiteral("FAIL"));
    normalizedVerification.insert(QStringLiteral("verified"), false);
    normalizedVerification.insert(QStringLiteral("finalizationStatus"),
                                  QStringLiteral("PENDING_CPP_MISSION_LIFECYCLE"));
    if (claimsReady) {
        QJsonArray failedChecks = normalizedVerification.value(QStringLiteral("failedChecks")).toArray();
        const QString blocker = QStringLiteral("CPP_MISSION_FINALIZATION_REQUIRED");
        if (!failedChecks.contains(blocker))
            failedChecks.append(blocker);
        normalizedVerification.insert(QStringLiteral("failedChecks"), failedChecks);
    }
    normalized.insert(QStringLiteral("verification"), normalizedVerification);

    invalidateLifecycleReadiness(QStringLiteral("PLANNING_RESULT_CHANGED_REQUIRES_CPP_FINALIZATION"));
    publishResult(normalized.toVariantMap());
    return true;
}

void PlanningBridge::consumeStdout()
{
    m_stdoutBuffer.append(m_process.readAllStandardOutput());

    while (true) {
        const qsizetype newline = m_stdoutBuffer.indexOf('\n');
        if (newline < 0)
            return;

        const QByteArray line = m_stdoutBuffer.left(newline).trimmed();
        m_stdoutBuffer.remove(0, newline + 1);

        if (line.isEmpty())
            continue;

        publishJson(QString::fromUtf8(line));
    }
}

void PlanningBridge::publishResult(const QVariantMap &result)
{
    if (m_result == result)
        return;

    m_result = result;
    emit resultChanged();
}

void PlanningBridge::clear()
{
    if (!m_result.isEmpty()) {
        m_result.clear();
        emit resultChanged();
    }
    invalidateLifecycleReadiness(QStringLiteral("CPP_MISSION_FINALIZATION_REQUIRED"));
}
