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
    Q_PROPERTY(QVariantList candidateSnapshots READ candidateSnapshots NOTIFY candidateSetChanged)

public:
    explicit RouteController(QObject *parent = nullptr) : QObject(parent) {}

    QVariantList waypoints() const { return m_waypoints; }
    QString routeId() const { return m_routeId; }
    QString routeVersion() const { return m_routeVersion; }
    QString calculationInputVersion() const { return m_calculationInputVersion; }
    QString validationState() const { return m_validationState; }
    QString validationReason() const { return m_validationReason; }
    QVariantList candidateSnapshots() const { return m_candidateSnapshots; }

    Q_INVOKABLE QVariantMap routeSnapshot() const
    {
        QVariantMap snapshot;
        snapshot.insert(QStringLiteral("routeId"), m_routeId);
        snapshot.insert(QStringLiteral("routeVersion"), m_routeVersion);
        snapshot.insert(QStringLiteral("missionId"), QString::fromStdString(m_route.lineage.mission_id));
        snapshot.insert(QStringLiteral("missionVersion"), QString::fromStdString(m_route.lineage.mission_version));
        snapshot.insert(QStringLiteral("generatorId"), QString::fromStdString(m_route.lineage.generator_id));
        snapshot.insert(QStringLiteral("generatorVersion"), QString::fromStdString(m_route.lineage.generator_version));
        snapshot.insert(QStringLiteral("calculationInputVersion"), m_calculationInputVersion);
        snapshot.insert(QStringLiteral("coordinateReferenceSystem"), QStringLiteral("EPSG:4326"));
        QVariantMap environmentSnapshotRefs;
        environmentSnapshotRefs.insert(QStringLiteral("terrainSnapshotId"),
                                        QString::fromStdString(m_route.environment.terrain_snapshot_id));
        environmentSnapshotRefs.insert(QStringLiteral("airspaceSnapshotId"),
                                        QString::fromStdString(m_route.environment.airspace_snapshot_id));
        environmentSnapshotRefs.insert(QStringLiteral("weatherSnapshotId"),
                                        QString::fromStdString(m_route.environment.weather_snapshot_id));
        environmentSnapshotRefs.insert(QStringLiteral("windSnapshotId"),
                                        QString::fromStdString(m_route.environment.wind_snapshot_id));
        snapshot.insert(QStringLiteral("environmentSnapshotRefs"), environmentSnapshotRefs);
        snapshot.insert(QStringLiteral("waypoints"), m_waypoints);
        QVariantList segmentSnapshot;
        for (const auto &segment : m_route.segments) {
            QVariantMap item{
                {QStringLiteral("segmentId"), QString::fromStdString(segment.segment_id)},
                {QStringLiteral("fromWaypointId"), QString::fromStdString(segment.from_waypoint_id)},
                {QStringLiteral("toWaypointId"), QString::fromStdString(segment.to_waypoint_id)},
                {QStringLiteral("distanceM"), segment.distance_m},
                {QStringLiteral("nominalSpeedMps"), segment.nominal_speed_mps}
            };
            if (segment.bank_angle_rad)
                item.insert(QStringLiteral("bankAngleRad"), *segment.bank_angle_rad);
            if (segment.lateral_acceleration_mps2)
                item.insert(QStringLiteral("lateralAccelerationMps2"), *segment.lateral_acceleration_mps2);
            segmentSnapshot.push_back(item);
        }
        snapshot.insert(QStringLiteral("segments"), segmentSnapshot);
        snapshot.insert(QStringLiteral("trajectoryGeometryComplete"),
                        m_route.trajectory_geometry_complete);
        snapshot.insert(QStringLiteral("validationState"), m_validationState);
        snapshot.insert(QStringLiteral("validationReason"), m_validationReason);
        return snapshot;
    }

    // Input is a projection of the canonical Route model, not an arbitrary QML
    // ListModel. Unknown/missing lineage and invalid waypoint data are rejected.
    Q_INVOKABLE bool loadCandidate(const QVariantMap &candidate)
    {
        m_candidateSnapshots.clear();
        emit candidateSetChanged();
        const QString routeId = candidate.value(QStringLiteral("routeId")).toString().trimmed();
        const QString routeVersion = candidate.value(QStringLiteral("routeVersion")).toString().trimmed();
        const QString missionId = candidate.value(QStringLiteral("missionId")).toString().trimmed();
        const QString missionVersion = candidate.value(QStringLiteral("missionVersion")).toString().trimmed();
        const QString generatorId = candidate.value(QStringLiteral("generatorId")).toString().trimmed();
        const QString generatorVersion = candidate.value(QStringLiteral("generatorVersion")).toString().trimmed();
        const QString inputVersion = candidate.value(QStringLiteral("calculationInputVersion")).toString().trimmed();
        const QString crs = candidate.value(QStringLiteral("coordinateReferenceSystem")).toString().trimmed();
        const QVariantMap environment = candidate.value(QStringLiteral("environmentSnapshotRefs")).toMap();
        const QString terrainSnapshotId = environment.value(QStringLiteral("terrainSnapshotId")).toString().trimmed();
        const QString airspaceSnapshotId = environment.value(QStringLiteral("airspaceSnapshotId")).toString().trimmed();
        const QString weatherSnapshotId = environment.value(QStringLiteral("weatherSnapshotId")).toString().trimmed();
        const QString windSnapshotId = environment.value(QStringLiteral("windSnapshotId")).toString().trimmed();
        const QVariantList points = candidate.value(QStringLiteral("waypoints")).toList();
        const QVariant completeValue =
            candidate.value(QStringLiteral("trajectoryGeometryComplete"));
        bool trajectoryGeometryComplete = false;
        if (completeValue.isValid()) {
            if (completeValue.metaType().id() != QMetaType::Bool)
                return rejectCandidate(QStringLiteral("TRAJECTORY_GEOMETRY_COMPLETENESS_FLAG_INVALID"));
            trajectoryGeometryComplete = completeValue.toBool();
        }
        const QVariantList segmentValues = candidate.value(QStringLiteral("segments")).toList();

        if (routeId.isEmpty() || routeVersion.isEmpty() || missionId.isEmpty()
            || missionVersion.isEmpty() || generatorId.isEmpty()
            || generatorVersion.isEmpty() || inputVersion.isEmpty() || crs.isEmpty()) {
            return rejectCandidate(QStringLiteral("ROUTE_LINEAGE_REQUIRED"));
        }
        if (terrainSnapshotId.isEmpty() || airspaceSnapshotId.isEmpty()) {
            return rejectCandidate(QStringLiteral("TERRAIN_AND_AIRSPACE_SNAPSHOT_LINEAGE_REQUIRED"));
        }
        // Route generation precedes weather assessment. Both weather references
        // may therefore be empty, but a partial weather lineage is never accepted.
        if (weatherSnapshotId.isEmpty() != windSnapshotId.isEmpty()) {
            return rejectCandidate(QStringLiteral("WEATHER_AND_WIND_SNAPSHOT_LINEAGE_MUST_BE_PAIRED"));
        }
        if (points.size() < 2)
            return rejectCandidate(QStringLiteral("AT_LEAST_TWO_WAYPOINTS_REQUIRED"));
        if (crs != QLatin1String("EPSG:4326"))
            return rejectCandidate(QStringLiteral("UNSUPPORTED_COORDINATE_REFERENCE_SYSTEM"));
        if (trajectoryGeometryComplete && segmentValues.size() != points.size() - 1)
            return rejectCandidate(QStringLiteral("COMPLETE_ROUTE_GEOMETRY_SEGMENTS_REQUIRED"));
        if (!segmentValues.isEmpty() && segmentValues.size() != points.size() - 1)
            return rejectCandidate(QStringLiteral("ROUTE_SEGMENT_COUNT_MISMATCH"));

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
            const QVariant mandatoryValue = point.value(QStringLiteral("mandatory"));
            if (id.isEmpty() || !ids.insert(id.toStdString()).second)
                return rejectCandidate(QStringLiteral("WAYPOINT_ID_MISSING_OR_DUPLICATE"));
            if (mandatoryValue.metaType().id() != QMetaType::Bool)
                return rejectCandidate(QStringLiteral("MANDATORY_FLAG_INVALID"));
            if (!latOk || !lonOk || !std::isfinite(lat) || !std::isfinite(lon)
                || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0)
                return rejectCandidate(QStringLiteral("INVALID_WAYPOINT_POSITION"));
            if (!altOk || !std::isfinite(alt) || alt < 0.0 || alt > 5000.0)
                return rejectCandidate(QStringLiteral("INVALID_WAYPOINT_ALTITUDE"));
            if (datum != QLatin1String("AGL"))
                return rejectCandidate(QStringLiteral("UNSUPPORTED_ALTITUDE_REFERENCE"));

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
        m_route.environment.terrain_snapshot_id = terrainSnapshotId.toStdString();
        m_route.environment.airspace_snapshot_id = airspaceSnapshotId.toStdString();
        m_route.environment.weather_snapshot_id = weatherSnapshotId.toStdString();
        m_route.environment.wind_snapshot_id = windSnapshotId.toStdString();
        std::set<std::string> segmentIds;
        for (qsizetype i = 0; i < segmentValues.size(); ++i) {
            const QVariantMap item = segmentValues.at(i).toMap();
            const QString segmentId = item.value(QStringLiteral("segmentId")).toString().trimmed();
            const QString fromId = item.value(QStringLiteral("fromWaypointId")).toString().trimmed();
            const QString toId = item.value(QStringLiteral("toWaypointId")).toString().trimmed();
            bool distanceOk = false;
            bool speedOk = false;
            const double distance = item.value(QStringLiteral("distanceM")).toDouble(&distanceOk);
            const double speed = item.value(QStringLiteral("nominalSpeedMps")).toDouble(&speedOk);
            const QVariantMap fromPoint = normalized.at(i).toMap();
            const QVariantMap toPoint = normalized.at(i + 1).toMap();
            if (segmentId.isEmpty() || !segmentIds.insert(segmentId.toStdString()).second)
                return rejectCandidate(QStringLiteral("ROUTE_SEGMENT_ID_MISSING_OR_DUPLICATE"));
            if (fromId != fromPoint.value(QStringLiteral("waypointId")).toString()
                || toId != toPoint.value(QStringLiteral("waypointId")).toString())
                return rejectCandidate(QStringLiteral("ROUTE_SEGMENT_CHAIN_DISCONTINUITY"));
            if (!distanceOk || !std::isfinite(distance) || distance <= 0.0
                || !speedOk || !std::isfinite(speed) || speed <= 0.0)
                return rejectCandidate(QStringLiteral("ROUTE_SEGMENT_PERFORMANCE_GEOMETRY_INVALID"));

            bluesky::planning::RouteSegment segment;
            segment.segment_id = segmentId.toStdString();
            segment.from_waypoint_id = fromId.toStdString();
            segment.to_waypoint_id = toId.toStdString();
            segment.distance_m = distance;
            segment.nominal_speed_mps = speed;

            const auto optionalFinite = [&item](const QString &key,
                                                 std::optional<double> &target) {
                if (!item.contains(key))
                    return true;
                bool ok = false;
                const double value = item.value(key).toDouble(&ok);
                if (!ok || !std::isfinite(value))
                    return false;
                target = value;
                return true;
            };
            if (!optionalFinite(QStringLiteral("bankAngleRad"), segment.bank_angle_rad)
                || !optionalFinite(QStringLiteral("lateralAccelerationMps2"),
                                   segment.lateral_acceleration_mps2))
                return rejectCandidate(QStringLiteral("ROUTE_SEGMENT_DYNAMIC_LIMIT_INVALID"));
            m_route.segments.push_back(std::move(segment));
        }
        m_route.trajectory_geometry_complete = trajectoryGeometryComplete;
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

    // Candidate sets are accepted only with complete canonical geometry and
    // explicit route and flight-safety validation evidence for every member.
    Q_INVOKABLE bool loadCandidateSet(const QVariantList &candidates,
                                      const QString &selectedCandidateId = QString())
    {
        m_candidateSnapshots.clear();
        emit candidateSetChanged();
        if (candidates.isEmpty())
            return rejectCandidate(QStringLiteral("WEATHER_CANDIDATE_SET_REQUIRED"));

        QVariantList validatedSnapshots;
        std::set<std::string> routeIds;
        std::set<std::string> candidateIds;
        QString commonMissionId, commonMissionVersion, commonInputVersion;
        QString commonSafetyEnvelopeId, commonSafetyVehicleId;
        QVariantMap selectedSnapshot;

        for (const QVariant &value : candidates) {
            const QVariantMap source = value.toMap();
            const QString candidateId = source.value(QStringLiteral("candidateId")).toString().trimmed();
            const QString safetyState = source.value(QStringLiteral("flightSafetyValidationState")).toString();
            const QString safetyEnvelopeId = source.value(QStringLiteral("flightSafetyEnvelopeId")).toString().trimmed();
            const QString safetyVehicleId = source.value(QStringLiteral("flightSafetyVehicleId")).toString().trimmed();
            if (source.value(QStringLiteral("validationState")).toString() != QLatin1String("VALIDATED"))
                return rejectCandidate(QStringLiteral("CANDIDATE_ROUTE_VALIDATION_NOT_CONFIRMED"));
            if (safetyState != QLatin1String("ALLOWED")
                || safetyEnvelopeId.isEmpty() || safetyVehicleId.isEmpty())
                return rejectCandidate(QStringLiteral("CANDIDATE_FLIGHT_SAFETY_VALIDATION_NOT_CONFIRMED"));
            if (candidateId.isEmpty() || !candidateIds.insert(candidateId.toStdString()).second)
                return rejectCandidate(QStringLiteral("CANDIDATE_ID_MISSING_OR_DUPLICATE"));

            bool timeOk = false, energyOk = false, reserveOk = false, scoreOk = false;
            const double nominalTime = source.value(QStringLiteral("estimatedTimeS")).toDouble(&timeOk);
            const double nominalEnergy = source.value(QStringLiteral("estimatedEnergyWh")).toDouble(&energyOk);
            const double nominalReserve = source.value(QStringLiteral("estimatedReserveWh")).toDouble(&reserveOk);
            const double objectiveScore = source.value(QStringLiteral("objectiveScore")).toDouble(&scoreOk);
            if (!timeOk || !energyOk || !reserveOk || !scoreOk
                || !std::isfinite(nominalTime) || nominalTime <= 0.0
                || !std::isfinite(nominalEnergy) || nominalEnergy < 0.0
                || !std::isfinite(nominalReserve) || nominalReserve < 0.0
                || !std::isfinite(objectiveScore))
                return rejectCandidate(QStringLiteral("CANDIDATE_NOMINAL_OBJECTIVE_METRICS_INVALID"));

            RouteController validator;
            if (!validator.loadCandidate(source))
                return rejectCandidate(QStringLiteral("CANDIDATE_ROUTE_INVALID: ") + validator.validationReason());
            if (!validator.routeSnapshot().value(QStringLiteral("trajectoryGeometryComplete")).toBool())
                return rejectCandidate(QStringLiteral("COMPLETE_TRAJECTORY_GEOMETRY_REQUIRED"));
            if (!validator.publishValidation(validator.routeVersion(), validator.calculationInputVersion(),
                                             true, QStringLiteral("OPTIMIZER_CANDIDATE_VALIDATION_ATTESTATION_MATCHED")))
                return rejectCandidate(QStringLiteral("CANDIDATE_VALIDATION_LINEAGE_MISMATCH"));

            QVariantMap snapshot = validator.routeSnapshot();
            const QString routeId = snapshot.value(QStringLiteral("routeId")).toString();
            const QString missionId = snapshot.value(QStringLiteral("missionId")).toString();
            const QString missionVersion = snapshot.value(QStringLiteral("missionVersion")).toString();
            const QString inputVersion = snapshot.value(QStringLiteral("calculationInputVersion")).toString();
            if (routeId.isEmpty() || !routeIds.insert(routeId.toStdString()).second)
                return rejectCandidate(QStringLiteral("ROUTE_ID_MISSING_OR_DUPLICATE"));
            if (commonMissionId.isEmpty()) {
                commonMissionId = missionId;
                commonMissionVersion = missionVersion;
                commonInputVersion = inputVersion;
                commonSafetyEnvelopeId = safetyEnvelopeId;
                commonSafetyVehicleId = safetyVehicleId;
            } else if (commonMissionId != missionId || commonMissionVersion != missionVersion
                       || commonInputVersion != inputVersion || commonSafetyEnvelopeId != safetyEnvelopeId
                       || commonSafetyVehicleId != safetyVehicleId) {
                return rejectCandidate(QStringLiteral("CANDIDATE_SET_MISSION_INPUT_OR_SAFETY_ENVELOPE_MISMATCH"));
            }

            snapshot.insert(QStringLiteral("candidateId"), candidateId);
            snapshot.insert(QStringLiteral("estimatedTimeS"), nominalTime);
            snapshot.insert(QStringLiteral("estimatedEnergyWh"), nominalEnergy);
            snapshot.insert(QStringLiteral("estimatedReserveWh"), nominalReserve);
            snapshot.insert(QStringLiteral("objectiveScore"), objectiveScore);
            snapshot.insert(QStringLiteral("flightSafetyValidationState"), safetyState);
            snapshot.insert(QStringLiteral("flightSafetyEnvelopeId"), safetyEnvelopeId);
            snapshot.insert(QStringLiteral("flightSafetyVehicleId"), safetyVehicleId);
            validatedSnapshots.push_back(snapshot);
            if (candidateId == selectedCandidateId || selectedSnapshot.isEmpty())
                selectedSnapshot = snapshot;
        }

        if (!selectedCandidateId.isEmpty()
            && selectedSnapshot.value(QStringLiteral("candidateId")).toString() != selectedCandidateId)
            return rejectCandidate(QStringLiteral("SELECTED_CANDIDATE_NOT_IN_VALIDATED_SET"));
        if (!loadCandidate(selectedSnapshot))
            return false;
        if (!publishValidation(routeVersion(), calculationInputVersion(), true,
                               QStringLiteral("OPTIMIZER_CANDIDATE_SET_VALIDATED")))
            return rejectCandidate(QStringLiteral("SELECTED_CANDIDATE_VALIDATION_LINEAGE_MISMATCH"));
        m_candidateSnapshots = validatedSnapshots;
        emit candidateSetChanged();
        return true;
    }

    Q_INVOKABLE bool selectCandidate(const QString &candidateId)
    {
        const QString requested = candidateId.trimmed();
        if (requested.isEmpty())
            return reject(QStringLiteral("CANDIDATE_ID_REQUIRED"));
        for (const QVariant &value : m_candidateSnapshots) {
            const QVariantMap snapshot = value.toMap();
            if (snapshot.value(QStringLiteral("candidateId")).toString() != requested)
                continue;
            const QVariantList preservedSet = m_candidateSnapshots;
            if (!loadCandidate(snapshot))
                return false;
            if (!publishValidation(routeVersion(), calculationInputVersion(), true,
                                   QStringLiteral("OPTIMIZER_CANDIDATE_SET_VALIDATED")))
                return rejectCandidate(QStringLiteral("SELECTED_CANDIDATE_VALIDATION_LINEAGE_MISMATCH"));
            m_candidateSnapshots = preservedSet;
            emit candidateSetChanged();
            return true;
        }
        return reject(QStringLiteral("CANDIDATE_NOT_FOUND"));
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
            // An altitude edit invalidates the previous flown-path/transition geometry.
            m_route.segments.clear();
            m_route.trajectory_geometry_complete = false;
            QVariantMap point = m_waypoints[i].toMap();
            point.insert(QStringLiteral("altitudeM"), altitudeM);
            m_waypoints[i] = point;
            ++m_editSequence;
            m_routeVersion = QString::fromStdString(m_route.lineage.route_version)
                             + QStringLiteral("-edit-") + QString::number(m_editSequence);
            m_route.lineage.route_version = m_routeVersion.toStdString();
            m_validationState = QStringLiteral("RECALCULATION_REQUIRED");
            m_validationReason = QStringLiteral("ROUTE_EDIT_INVALIDATED_PREVIOUS_VALIDATION");
            m_candidateSnapshots.clear();
            emit candidateSetChanged();
            emit routeChanged();
            emit validationChanged();
            return true;
        }
        return reject(QStringLiteral("WAYPOINT_NOT_FOUND"));
    }

    // A rejected replacement candidate must not leave the previous route visible.
    bool rejectCandidate(const QString &reason)
    {
        m_route = {};
        m_waypoints.clear();
        m_candidateSnapshots.clear();
        emit candidateSetChanged();
        m_routeId.clear();
        m_routeVersion.clear();
        m_calculationInputVersion.clear();
        m_validationState = QStringLiteral("BLOCKED");
        m_validationReason = reason;
        m_editSequence = 0;
        emit routeChanged();
        emit validationChanged();
        return false;
    }

    // Planner validation is accepted only for the exact route and input versions.
    Q_INVOKABLE bool publishValidation(const QString &validatedRouteVersion,
                                       const QString &validatedInputVersion,
                                       bool accepted,
                                       const QString &reason)
    {
        if (m_route.waypoints.empty())
            return reject(QStringLiteral("NO_CANDIDATE_ROUTE"));
        if (validatedRouteVersion != m_routeVersion
            || validatedInputVersion != m_calculationInputVersion) {
            // Ignore late planner replies. They must not overwrite the state or
            // reason associated with a newer, already accepted validation.
            return false;
        }
        m_validationState = accepted ? QStringLiteral("VALIDATED")
                                     : QStringLiteral("BLOCKED");
        m_validationReason = reason.trimmed().isEmpty()
            ? (accepted ? QStringLiteral("PLANNER_VALIDATION_MATCHED")
                        : QStringLiteral("PLANNER_REJECTED_ROUTE"))
            : reason.trimmed();
        for (QVariant &value : m_candidateSnapshots) {
            QVariantMap item = value.toMap();
            if (item.value(QStringLiteral("routeId")).toString() == m_routeId) {
                item.insert(QStringLiteral("validationState"), m_validationState);
                item.insert(QStringLiteral("validationReason"), m_validationReason);
                value = item;
                emit candidateSetChanged();
                break;
            }
        }
        emit validationChanged();
        return true;
    }

    Q_INVOKABLE void clear()
    {
        m_route = {};
        m_waypoints.clear();
        m_candidateSnapshots.clear();
        emit candidateSetChanged();
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
    void candidateSetChanged();

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
    QVariantList m_candidateSnapshots;
    QString m_routeId;
    QString m_routeVersion;
    QString m_calculationInputVersion;
    QString m_validationState{QStringLiteral("EMPTY")};
    QString m_validationReason{QStringLiteral("NO_ROUTE")};
    quint64 m_editSequence{0};
};
