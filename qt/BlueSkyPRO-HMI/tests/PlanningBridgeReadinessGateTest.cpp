#include "PlanningBridge.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

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

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    testExternalPassCannotGrantReady();
    testExistingBlockersArePreserved();
    return 0;
}
