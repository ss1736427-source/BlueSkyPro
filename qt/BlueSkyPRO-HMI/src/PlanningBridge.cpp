#include "PlanningBridge.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>

namespace
{
constexpr auto kSchemaVersion = "1.0";
constexpr auto kMessageType = "planning.result";
}

PlanningBridge::PlanningBridge(QObject *parent)
    : QObject(parent)
{
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

    QByteArray payload = json.toUtf8();
    if (!payload.endsWith('\n'))
        payload.append('\n');

    // Queue the JSONL frame and return immediately. Waiting synchronously for
    // bytes to leave QProcess can block the caller (normally the GUI thread).
    // QProcess drains its write buffer asynchronously; report only whether
    // the frame was accepted into that buffer.
    const qint64 queued = m_process.write(payload);
    if (queued != payload.size()) {
        emit bridgeError(QStringLiteral("PLANNING_REQUEST_QUEUE_FAILED"));
        return false;
    }

    return true;
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

    const QJsonObject resultObject = object.value(QStringLiteral("result")).toObject();
    if (resultObject.contains(QStringLiteral("routeGeometry"))) {
        const QJsonObject route = resultObject.value(QStringLiteral("routeGeometry")).toObject();
        if (route.value(QStringLiteral("routeId")).toString().isEmpty()
            || route.value(QStringLiteral("routeVersion")).toString().isEmpty()
            || route.value(QStringLiteral("coordinateReference")).toString() != QLatin1String("WGS84")) {
            emit bridgeError(QStringLiteral("INVALID_ROUTE_GEOMETRY_METADATA"));
            return false;
        }

        const QJsonArray points = route.value(QStringLiteral("points")).toArray();
        if (points.size() < 2) {
            emit bridgeError(QStringLiteral("ROUTE_REQUIRES_AT_LEAST_TWO_POINTS"));
            return false;
        }

        for (const QJsonValue &value : points) {
            if (!value.isObject()) {
                emit bridgeError(QStringLiteral("INVALID_ROUTE_POINT"));
                return false;
            }
            const QJsonObject point = value.toObject();
            const double latitude = point.value(QStringLiteral("latitude")).toDouble(qQNaN());
            const double longitude = point.value(QStringLiteral("longitude")).toDouble(qQNaN());
            if (point.value(QStringLiteral("waypointId")).toString().isEmpty()
                || !qIsFinite(latitude) || latitude < -90.0 || latitude > 90.0
                || !qIsFinite(longitude) || longitude < -180.0 || longitude > 180.0
                || !point.value(QStringLiteral("altitudeM")).isDouble()
                || !point.value(QStringLiteral("mandatory")).isBool()) {
                emit bridgeError(QStringLiteral("INVALID_ROUTE_POINT_FIELDS"));
                return false;
            }
        }
    }

    publishResult(object.toVariantMap());
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
    if (m_result.isEmpty())
        return;

    m_result.clear();
    emit resultChanged();
}
