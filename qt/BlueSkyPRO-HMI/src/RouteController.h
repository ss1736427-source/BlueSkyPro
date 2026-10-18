#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

#include <cmath>
#include <set>
#include <string>

#include "04_SOFTWARE/PLANNING/model/route_model.hpp"

// HMI-facing controller for the canonical planning Route value.
// Editing creates an unvalidated candidate; only a matching planner result can
// move it to VALIDATED. This controller does not itself authorize flight.
class RouteController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList waypoints READ waypoints NOTIFY routeChanged)
    Q_PROPERTY(QString routeId READ routeId NOTIFY routeChanged)
    Q_PROPERTY(QString routeVersion READ routeVersion NOTIFY routeChanged)
    Q_PROPERTY(QString calculationInputVersion READ calculationInputVersion NOTIFY routeChanged)
    Q_PROPERTY(QString validationState READ validationState NOTIFY validationChanged)
    Q_PROPERTY(QString validationReason READ validationReason NOTIFY validationChanged)

public:
    explicit RouteController(QObject *parent = nullptr) : QObject(parent) {}

    QVariantList waypoints() const { return m_waypoints; }
    QString routeId() const { return m_routeId; }
    QString routeVersion() const { return m_routeVersion; }
    QString calculationInputVersion() const { return m_calculationInputVersion; }
    QString validationState() const { return m_validationState; }
    QString validationReason() const { return m_validationReason; }

    // Input is a projection of the canonical Route model, not an arbitrary QML
    // ListModel. Unknown/missing lineage and invalid waypoint data are rejected.
    Q_INVOKABLE bool loadCandidate(const QVariantMap &candidate)
    {
        const QString routeId = candidate.value(QStringLiteral("routeId")).toString().trimmed();
        const QString routeVersion = candidate.value(QStringLiteral("routeVersion")).toString().trimmed();
        const QString missionId = candidate.value(QStringLiteral("missionId")).toString().trimmed();
        const QString missionVersion = candidate.value(QStringLiteral("missionVersion")).toString().trimmed();
        const QString generatorId = candidate.value(QStringLiteral("generatorId")).toString().trimmed();
        const QString generatorVersion = candidate.value(QStringLiteral("generatorVersion")).toString().trimmed();
        const QString inputVersion = candidate.value(QStringLiteral("calculationInputVersion")).toString().trimmed();
        const QVariantList points = candidate.value(QStringLiteral("waypoints")).toList();

        if (routeId.isEmpty() || routeVersion.isEmpty() || missionId.isEmpty()
            || missionVersion.isEmpty() || generatorId.isEmpty()
            || generatorVersion.isEmpty() || inputVersion.isEmpty()) {
            return reject(QStringLiteral("ROUTE_LINEAGE_REQUIRED"));
        }
        if (points.size() < 2)
            return reject(QStringLiteral("AT_LEAST_TWO_WAYPOINTS_REQUIRED"));

        std::set<std::string> ids;
        QVariantList normalized;
        for (const QVariant &value : points) {
            const QVariantMap point = value.toMap();
            const QString id = point.value(QStringLiteral("waypointId")).toString().trimmed();
            bool latOk = false, lonOk = false, altOk = false;
            const double lat = point.value(QStringLiteral("latitudeDeg")).toDouble(&latOk);
            const double lon = point.value(QStringLiteral("longitudeDeg")).toDouble(&lonOk);
            const double alt = point.value(QStringLiteral("altitudeM")).toDouble(&altOk);
            const QString datum = point.value(QStringLiteral("altitudeReference")).toString();
            if (id.isEmpty() || !ids.insert(id.toStdString()).second)
                return reject(QStringLiteral("WAYPOINT_ID_MISSING_OR_DUPLICATE"));
            if (!latOk || !lonOk || !std::isfinite(lat) || !std::isfinite(lon)
                || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0)
                return reject(QStringLiteral("INVALID_WAYPOINT_POSITION"));
            if (!altOk || !std::isfinite(alt) || alt < 0.0 || alt > 5000.0)
                return reject(QStringLiteral("INVALID_WAYPOINT_ALTITUDE"));
            if (datum != QLatin1String("AGL"))
                return reject(QStringLiteral("UNSUPPORTED_ALTITUDE_REFERENCE"));

            QVariantMap normalizedPoint;
            normalizedPoint.insert(QStringLiteral("waypointId"), id);
            normalizedPoint.insert(QStringLiteral("latitudeDeg"), lat);
            normalizedPoint.insert(QStringLiteral("longitudeDeg"), lon);
            normalizedPoint.insert(QStringLiteral("altitudeM"), alt);
            normalizedPoint.insert(QStringLiteral("mandatory"),
                                   point.value(QStringLiteral("mandatory")).toBool());
            normalizedPoint.insert(QStringLiteral("altitudeReference"), datum);
            normalized.append(normalizedPoint);
        }

        m_route = {};
        m_route.lineage.route_id = routeId.toStdString();
        m_route.lineage.route_version = routeVersion.toStdString();
        m_route.lineage.mission_id = missionId.toStdString();
        m_route.lineage.mission_version = missionVersion.toStdString();
        m_route.lineage.generator_id = generatorId.toStdString();
        m_route.lineage.generator_version = generatorVersion.toStdString();
        m_route.lineage.calculation_input_version = inputVersion.toStdString();
        for (const QVariant &value : normalized) {
            const QVariantMap point = value.toMap();
            bluesky::planning::RouteWaypoint waypoint;
            waypoint.waypoint_id = point.value(QStringLiteral("waypointId")).toString().toStdString();
            waypoint.position.latitude_deg = point.value(QStringLiteral("latitudeDeg")).toDouble();
            waypoint.position.longitude_deg = point.value(QStringLiteral("longitudeDeg")).toDouble();
            waypoint.altitude_m = point.value(QStringLiteral("altitudeM")).toDouble();
            waypoint.mandatory = point.value(QStringLiteral("mandatory")).toBool();
            waypoint.altitude_reference = bluesky::planning::AltitudeReference::AGL;
            m_route.waypoints.push_back(waypoint);
        }

        m_routeId = routeId;
        m_routeVersion = routeVersion;
        m_calculationInputVersion = inputVersion;
        m_waypoints = normalized;
        m_editSequence = 0;
        m_validationState = QStringLiteral("CANDIDATE_UNVALIDATED");
        m_validationReason = QStringLiteral("PLANNER_REVALIDATION_REQUIRED");
        emit routeChanged();
        emit validationChanged();
        return true;
    }

    Q_INVOKABLE bool editWaypointAltitude(const QString &waypointId, double altitudeM)
    {
        if (m_route.waypoints.empty())
            return reject(QStringLiteral("NO_CANDIDATE_ROUTE"));
        if (!std::isfinite(altitudeM) || altitudeM < 0.0 || altitudeM > 5000.0)
            return reject(QStringLiteral("INVALID_WAYPOINT_ALTITUDE"));

        for (size_t i = 0; i < m_route.waypoints.size(); ++i) {
            auto &waypoint = m_route.waypoints[i];
            if (waypoint.waypoint_id != waypointId.toStdString())
                continue;
            waypoint.altitude_m = altitudeM;
            QVariantMap point = m_waypoints[i].toMap();
            point.insert(QStringLiteral("altitudeM"), altitudeM);
            m_waypoints[i] = point;
            ++m_editSequence;
            m_routeVersion = QString::fromStdString(m_route.lineage.route_version)
                             + QStringLiteral("-edit-") + QString::number(m_editSequence);
            m_route.lineage.route_version = m_routeVersion.toStdString();
            m_validationState = QStringLiteral("RECALCULATION_REQUIRED");
            m_validationReason = QStringLiteral("ROUTE_EDIT_INVALIDATED_PREVIOUS_VALIDATION");
            emit routeChanged();
            emit validationChanged();
            return true;
        }
        return reject(QStringLiteral("WAYPOINT_NOT_FOUND"));
    }

    // Planner validation is accepted only for the exact route and input versions.
    bool publishValidation(const QString &validatedRouteVersion,
                                       const QString &validatedInputVersion,
                                       bool accepted,
                                       const QString &reason)
    {
        if (m_route.waypoints.empty())
            return reject(QStringLiteral("NO_CANDIDATE_ROUTE"));
        if (validatedRouteVersion != m_routeVersion
            || validatedInputVersion != m_calculationInputVersion) {
            m_validationReason = QStringLiteral("STALE_VALIDATION_RESULT");
            emit validationChanged();
            return false;
        }
        m_validationState = accepted ? QStringLiteral("VALIDATED")
                                     : QStringLiteral("BLOCKED");
        m_validationReason = reason.trimmed().isEmpty()
            ? (accepted ? QStringLiteral("PLANNER_VALIDATION_MATCHED")
                        : QStringLiteral("PLANNER_REJECTED_ROUTE"))
            : reason.trimmed();
        emit validationChanged();
        return true;
    }

    Q_INVOKABLE void clear()
    {
        m_route = {};
        m_waypoints.clear();
        m_routeId.clear();
        m_routeVersion.clear();
        m_calculationInputVersion.clear();
        m_validationState = QStringLiteral("EMPTY");
        m_validationReason = QStringLiteral("NO_ROUTE");
        m_editSequence = 0;
        emit routeChanged();
        emit validationChanged();
    }

signals:
    void routeChanged();
    void validationChanged();

private:
    bool reject(const QString &reason)
    {
        m_validationState = QStringLiteral("BLOCKED");
        m_validationReason = reason;
        emit validationChanged();
        return false;
    }

    bluesky::planning::Route m_route;
    QVariantList m_waypoints;
    QString m_routeId;
    QString m_routeVersion;
    QString m_calculationInputVersion;
    QString m_validationState{QStringLiteral("EMPTY")};
    QString m_validationReason{QStringLiteral("NO_ROUTE")};
    quint64 m_editSequence{0};
};
