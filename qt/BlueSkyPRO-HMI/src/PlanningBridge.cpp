#include "PlanningBridge.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace
{
constexpr auto kSchemaVersion = "1.0";
constexpr auto kMessageType = "planning.result";
}

PlanningBridge::PlanningBridge(QObject *parent)
    : QObject(parent)
{
}

QVariantMap PlanningBridge::result() const
{
    return m_result;
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

    publishResult(object.toVariantMap());
    return true;
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
    if (m_result.isEmpty())
        return;

    m_result.clear();
    emit resultChanged();
}
