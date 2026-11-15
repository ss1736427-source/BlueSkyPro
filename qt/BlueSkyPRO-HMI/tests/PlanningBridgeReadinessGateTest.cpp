#include "PlanningBridge.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QVariantList>

#include <algorithm>
#include <cassert>

static QString resultMessage(const QString &releaseStatus,
                             const QString &finalGateStatus,
                             bool verified,
                             const QJsonArray &failedChecks = {})
{
    QJsonObject candidate{
        {QStringLiteral("routeId"), QStringLiteral("ROUTE-1")},
        {QStringLiteral("routeVersion"), QStringLiteral("4")},
        {QStringLiteral("missionId"), QStringLiteral("MISSION-1")},
        {QStringLiteral("waypoints"), QJsonArray{QJsonObject{{QStringLiteral("waypointId"),
                                                             QStringLiteral("WP-1")}}}}
    };
    QJsonObject root{
        {QStringLiteral("schemaVersion"), QStringLiteral("1.0")},
        {QStringLiteral("messageType"), QStringLiteral("planning.result")},
        {QStringLiteral("missionId"), QStringLiteral("MISSION-1")},
        {QStringLiteral("resultId"), QStringLiteral("RESULT-1")},
        {QStringLiteral("verification"), QJsonObject{
            {QStringLiteral("releaseStatus"), releaseStatus},
            {QStringLiteral("finalGateStatus"), finalGateStatus},
            {QStringLiteral("verified"), verified},
            {QStringLiteral("failedChecks"), failedChecks}
        }},
        {QStringLiteral("result"), QJsonObject{
            {QStringLiteral("routeCandidate"), candidate}
        }}
    };
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

static void testExternalPassCannotGrantReady()
{
    PlanningBridge bridge;
    assert(bridge.publishJson(resultMessage(QStringLiteral("RELEASE_ELIGIBLE"),
                                            QStringLiteral("PASS"), true)));

    const QVariantMap published = bridge.result();
    const QVariantMap verification = published.value(QStringLiteral("verification")).toMap();
    assert(verification.value(QStringLiteral("releaseStatus")).toString() == QStringLiteral("BLOCKED"));
    assert(verification.value(QStringLiteral("finalGateStatus")).toString() == QStringLiteral("FAIL"));
    assert(!verification.value(QStringLiteral("verified")).toBool());
    assert(verification.value(QStringLiteral("finalizationStatus")).toString()
           == QStringLiteral("PENDING_CPP_MISSION_LIFECYCLE"));

    const QStringList failedChecks = verification.value(QStringLiteral("failedChecks")).toStringList();
    assert(failedChecks.contains(QStringLiteral("CPP_MISSION_FINALIZATION_REQUIRED")));

    // The planner's candidate remains available for visualization and review.
    const QVariantMap result = published.value(QStringLiteral("result")).toMap();
    assert(result.value(QStringLiteral("routeCandidate")).toMap()
               .value(QStringLiteral("routeId")).toString() == QStringLiteral("ROUTE-1"));
}

static void testExistingBlockersArePreserved()
{
    PlanningBridge bridge;
    const QJsonArray upstream{QStringLiteral("WEATHER_EVIDENCE_MISSING")};
    assert(bridge.publishJson(resultMessage(QStringLiteral("BLOCKED"),
                                            QStringLiteral("FAIL"), false, upstream)));

    const QVariantMap verification = bridge.result().value(QStringLiteral("verification")).toMap();
    assert(verification.value(QStringLiteral("releaseStatus")).toString() == QStringLiteral("BLOCKED"));
    assert(verification.value(QStringLiteral("finalGateStatus")).toString() == QStringLiteral("FAIL"));
    assert(verification.value(QStringLiteral("failedChecks")).toStringList()
               .contains(QStringLiteral("WEATHER_EVIDENCE_MISSING")));
    assert(verification.value(QStringLiteral("finalizationStatus")).toString()
           == QStringLiteral("PENDING_CPP_MISSION_LIFECYCLE"));
}


static bluesky::planning::MissionLifecycleRecord readyRecord()
{
    using namespace bluesky::planning;
    MissionLifecycleRecord record;
    record.execution_mode = MissionExecutionMode::Virtual;
    record.mission_id = "MISSION-1";
    record.mission_version = "2";
    record.current_route_id = "ROUTE-1";
    record.current_route_version = "4";
    record.current_calculation_input_version = "INPUT-9";
    record.vehicle_id = "UAV-SIL-TEST";
    record.vehicle_source_provenance = "SIL_FIXTURE";
    record.reference_model_id = "TEST-MODEL";
    record.vehicle_configuration_id = "CFG-TEST";
    record.vehicle_configuration_revision = "1";
    record.performance_profile_id = "PERF-TEST";
    record.performance_profile_revision = "1";
    record.state = MissionLifecycleState::Ready;
    MissionLifecycleEvent event;
    event.execution_mode = record.execution_mode;
    event.sequence = 1;
    event.mission_id = record.mission_id;
    event.mission_version = record.mission_version;
    event.actor_id = "operator-1";
    event.timestamp = "2026-11-15T00:00:00Z";
    event.from = MissionLifecycleState::Validating;
    event.to = MissionLifecycleState::Ready;
    event.route_id = record.current_route_id;
    event.route_version = record.current_route_version;
    event.evidence_reference = record.current_calculation_input_version;
    event.vehicle_id = record.vehicle_id;
    event.vehicle_source_provenance = record.vehicle_source_provenance;
    event.reference_model_id = record.reference_model_id;
    event.vehicle_configuration_id = record.vehicle_configuration_id;
    event.vehicle_configuration_revision = record.vehicle_configuration_revision;
    event.performance_profile_id = record.performance_profile_id;
    event.performance_profile_revision = record.performance_profile_revision;
    record.history.push_back(event);
    record.next_event_sequence = 2;
    return record;
}

static void testChecklistContainsOnlyPilotActions()
{
    using namespace bluesky::planning;
    PlanningBridge bridge;
    auto record = readyRecord();
    MissionLifecycleTransitionResult transition;
    transition.accepted = true;
    transition.state = MissionLifecycleState::Ready;
    const QVariantList actions{
        QVariantMap{{QStringLiteral("actionId"), QStringLiteral("automatic_weather_refresh")},
                    {QStringLiteral("label"), QStringLiteral("Обновить погоду")},
                    {QStringLiteral("requiresHuman"), false}},
        QVariantMap{{QStringLiteral("actionId"), QStringLiteral("pilot_review")},
                    {QStringLiteral("label"), QStringLiteral("Подтвердить проверку пилотом")},
                    {QStringLiteral("owner"), QStringLiteral("pilot")},
                    {QStringLiteral("requiresHuman"), true}}
    };
    assert(bridge.publishLifecycleReadiness(record, transition, actions));
    const QVariantMap snapshot = bridge.readinessSnapshot();
    assert(snapshot.value(QStringLiteral("ready")).toBool());
    assert(snapshot.value(QStringLiteral("vehicleSourceProvenance")).toString() == QStringLiteral("SIL_FIXTURE"));
    assert(snapshot.value(QStringLiteral("vehicleId")).toString() == QStringLiteral("UAV-SIL-TEST"));
    assert(!snapshot.value(QStringLiteral("released")).toBool());
    const QVariantList pilotActions = snapshot.value(QStringLiteral("pilotActions")).toList();
    assert(pilotActions.size() == 1);
    assert(pilotActions.front().toMap().value(QStringLiteral("actionId")).toString()
           == QStringLiteral("pilot_review"));
}

static void testVirtualReadinessPublishesDirectSandboxRelease()
{
    using namespace bluesky::planning;
    PlanningBridge bridge;
    auto record = readyRecord();
    MissionLifecycleEvent released;
    released.execution_mode = MissionExecutionMode::Virtual;
    released.sequence = 2;
    released.mission_id = record.mission_id;
    released.mission_version = record.mission_version;
    released.actor_id = "operator-1";
    released.timestamp = "2026-11-15T00:01:00Z";
    released.from = MissionLifecycleState::Ready;
    released.to = MissionLifecycleState::Released;
    released.route_id = record.current_route_id;
    released.route_version = record.current_route_version;
    released.evidence_reference = record.current_calculation_input_version;
    released.vehicle_id = record.vehicle_id;
    released.vehicle_source_provenance = record.vehicle_source_provenance;
    released.reference_model_id = record.reference_model_id;
    released.vehicle_configuration_id = record.vehicle_configuration_id;
    released.vehicle_configuration_revision = record.vehicle_configuration_revision;
    released.performance_profile_id = record.performance_profile_id;
    released.performance_profile_revision = record.performance_profile_revision;
    record.history.push_back(released);
    record.next_event_sequence = 3;
    record.state = MissionLifecycleState::Released;

    MissionLifecycleTransitionResult transition;
    transition.accepted = true;
    transition.state = MissionLifecycleState::Released;
    assert(bridge.publishLifecycleReadiness(record, transition, {}));
    const QVariantMap snapshot = bridge.readinessSnapshot();
    assert(snapshot.value(QStringLiteral("executionMode")).toString() == QStringLiteral("VIRTUAL"));
    assert(snapshot.value(QStringLiteral("ready")).toBool());
    assert(snapshot.value(QStringLiteral("released")).toBool());
}

static void testLifecycleReadinessRejectsVehicleProvenanceDrift()
{
    using namespace bluesky::planning;
    PlanningBridge bridge;
    auto record = readyRecord();
    record.history.front().vehicle_source_provenance = "PHYSICAL_LIVE";
    MissionLifecycleTransitionResult transition;
    transition.accepted = true;
    transition.state = MissionLifecycleState::Ready;
    assert(!bridge.publishLifecycleReadiness(record, transition, {}));
    const QVariantMap snapshot = bridge.readinessSnapshot();
    assert(!snapshot.value(QStringLiteral("ready")).toBool());
    assert(snapshot.value(QStringLiteral("blockers")).toList().contains(
        QStringLiteral("CPP_LIFECYCLE_SNAPSHOT_INVALID")));
}

static void testStaleLifecycleLineageCannotPublishReady()
{
    using namespace bluesky::planning;
    PlanningBridge bridge;
    auto record = readyRecord();
    record.current_route_version = "5";
    MissionLifecycleTransitionResult transition;
    transition.accepted = true;
    transition.state = MissionLifecycleState::Ready;
    assert(!bridge.publishLifecycleReadiness(record, transition, {}));
    const QVariantMap snapshot = bridge.readinessSnapshot();
    assert(!snapshot.value(QStringLiteral("ready")).toBool());
    assert(snapshot.value(QStringLiteral("blockers")).toList().contains(
        QStringLiteral("CPP_LIFECYCLE_SNAPSHOT_INVALID")));
}

static void testValidatingAssessmentShowsOnlyPilotActionsWithoutGrantingReady()
{
    using namespace bluesky::planning;
    PlanningBridge bridge;
    auto record = readyRecord();
    record.state = MissionLifecycleState::Validating;
    record.history.back().from = MissionLifecycleState::Calculating;
    record.history.back().to = MissionLifecycleState::Validating;

    MissionLifecycleTransitionResult assessment;
    assessment.accepted = false;
    assessment.state = MissionLifecycleState::Validating;
    assessment.blockers = {"NOTAM_VALIDATION_EVIDENCE_REQUIRED",
                           "OPERATOR_REVIEW_NOT_ACCEPTED"};

    const QVariantList actions{
        QVariantMap{{QStringLiteral("actionId"), QStringLiteral("automatic_notam_check")},
                    {QStringLiteral("label"), QStringLiteral("Автоматически проверить NOTAM")},
                    {QStringLiteral("requiresHuman"), false}},
        QVariantMap{{QStringLiteral("actionId"), QStringLiteral("pilot_review")},
                    {QStringLiteral("label"), QStringLiteral("Подтвердить проверку пилотом")},
                    {QStringLiteral("requiresHuman"), true}}
    };
    assert(bridge.publishLifecycleAssessment(record, assessment, actions));

    const QVariantMap snapshot = bridge.readinessSnapshot();
    assert(snapshot.value(QStringLiteral("lifecycleState")).toString()
           == QStringLiteral("VALIDATING"));
    assert(!snapshot.value(QStringLiteral("ready")).toBool());
    assert(!snapshot.value(QStringLiteral("released")).toBool());
    assert(snapshot.value(QStringLiteral("blockers")).toList().size() == 2);
    const QVariantList pilotActions = snapshot.value(QStringLiteral("pilotActions")).toList();
    assert(pilotActions.size() == 1);
    assert(pilotActions.front().toMap().value(QStringLiteral("actionId")).toString()
           == QStringLiteral("pilot_review"));
}

static void testValidatingAssessmentRejectsStaleLineage()
{
    using namespace bluesky::planning;
    PlanningBridge bridge;
    auto record = readyRecord();
    record.state = MissionLifecycleState::Validating;
    record.history.back().from = MissionLifecycleState::Calculating;
    record.history.back().to = MissionLifecycleState::Validating;
    record.history.back().route_version = "stale-route";

    MissionLifecycleTransitionResult assessment;
    assessment.accepted = false;
    assessment.state = MissionLifecycleState::Validating;
    assessment.blockers = {"OPERATOR_REVIEW_NOT_ACCEPTED"};

    assert(!bridge.publishLifecycleAssessment(record, assessment, {}));
    const QVariantMap snapshot = bridge.readinessSnapshot();
    assert(!snapshot.value(QStringLiteral("ready")).toBool());
    assert(snapshot.value(QStringLiteral("blockers")).toList().contains(
        QStringLiteral("CPP_LIFECYCLE_ASSESSMENT_INVALID")));
}

static void testExternalPlanningResultInvalidatesLifecycleSnapshot()
{
    using namespace bluesky::planning;
    PlanningBridge bridge;
    auto record = readyRecord();
    MissionLifecycleTransitionResult transition;
    transition.accepted = true;
    transition.state = MissionLifecycleState::Ready;
    assert(bridge.publishLifecycleReadiness(record, transition, {}));
    assert(bridge.readinessSnapshot().value(QStringLiteral("ready")).toBool());
    assert(bridge.publishJson(resultMessage(QStringLiteral("BLOCKED"),
                                            QStringLiteral("FAIL"), false)));
    const QVariantMap snapshot = bridge.readinessSnapshot();
    assert(!snapshot.value(QStringLiteral("ready")).toBool());
    assert(snapshot.value(QStringLiteral("blockers")).toList().contains(
        QStringLiteral("PLANNING_RESULT_CHANGED_REQUIRES_CPP_FINALIZATION")));
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    testExternalPassCannotGrantReady();
    testExistingBlockersArePreserved();
    testChecklistContainsOnlyPilotActions();
    testVirtualReadinessPublishesDirectSandboxRelease();
    testLifecycleReadinessRejectsVehicleProvenanceDrift();
    testStaleLifecycleLineageCannotPublishReady();
    testValidatingAssessmentShowsOnlyPilotActionsWithoutGrantingReady();
    testValidatingAssessmentRejectsStaleLineage();
    testExternalPlanningResultInvalidatesLifecycleSnapshot();
    return 0;
}
