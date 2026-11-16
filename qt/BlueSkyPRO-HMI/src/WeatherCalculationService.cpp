#include "WeatherCalculationService.h"

#include "04_SOFTWARE/PLANNING/model/route_model.hpp"
#include "04_SOFTWARE/PLANNING/wind_performance_trajectory.hpp"
#include "04_SOFTWARE/WEATHER/grib2_weather_pipeline.hpp"
#include "04_SOFTWARE/WEATHER/weather_candidate_assessor.hpp"
#include "04_SOFTWARE/PLANNING/orchestrator/algorithm_orchestrator.hpp"
#include "04_SOFTWARE/PLANNING/notam_prohibited_zone_validator.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QSet>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent/QtConcurrentRun>

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {
using bluesky::planning::AltitudeReference;
using bluesky::planning::Route;
using bluesky::planning::RouteSegment;
using bluesky::planning::RouteWaypoint;
using bluesky::planning::TrajectoryPerformanceInput;

QString jsonString(const QJsonObject& object,const char* camel,const char* snake=nullptr)
{
    const auto first=object.value(QLatin1String(camel));
    if (first.isString()) return first.toString();
    return snake ? object.value(QLatin1String(snake)).toString() : QString();
}

bool readFinite(const QVariantMap& map,const QString& key,double& value)
{
    bool ok=false;
    value=map.value(key).toDouble(&ok);
    return ok && std::isfinite(value);
}

bool buildRoute(const QVariantMap& snapshot,Route& route,QString& error)
{
    route.lineage.mission_id=snapshot.value(QStringLiteral("missionId")).toString().toStdString();
    route.lineage.mission_version=snapshot.value(QStringLiteral("missionVersion")).toString().toStdString();
    route.lineage.route_id=snapshot.value(QStringLiteral("routeId")).toString().toStdString();
    route.lineage.route_version=snapshot.value(QStringLiteral("routeVersion")).toString().toStdString();
    route.lineage.calculation_input_version=
        snapshot.value(QStringLiteral("calculationInputVersion")).toString().toStdString();
    if (route.lineage.mission_id.empty() || route.lineage.mission_version.empty() || route.lineage.route_id.empty()
        || route.lineage.route_version.empty() || route.lineage.calculation_input_version.empty()) {
        error=QStringLiteral("CANONICAL_ROUTE_LINEAGE_INCOMPLETE");
        return false;
    }
    const QVariantMap environment = snapshot.value(QStringLiteral("environmentSnapshotRefs")).toMap();
    route.environment.terrain_snapshot_id =
        environment.value(QStringLiteral("terrainSnapshotId")).toString().trimmed().toStdString();
    route.environment.airspace_snapshot_id =
        environment.value(QStringLiteral("airspaceSnapshotId")).toString().trimmed().toStdString();
    route.environment.weather_snapshot_id =
        environment.value(QStringLiteral("weatherSnapshotId")).toString().trimmed().toStdString();
    route.environment.wind_snapshot_id =
        environment.value(QStringLiteral("windSnapshotId")).toString().trimmed().toStdString();
    if (route.environment.terrain_snapshot_id.empty()
        || route.environment.airspace_snapshot_id.empty()) {
        error=QStringLiteral("TERRAIN_AND_AIRSPACE_SNAPSHOT_LINEAGE_REQUIRED");
        return false;
    }
    if (route.environment.weather_snapshot_id.empty()
        != route.environment.wind_snapshot_id.empty()) {
        error=QStringLiteral("WEATHER_AND_WIND_SNAPSHOT_LINEAGE_MUST_BE_PAIRED");
        return false;
    }
    if (snapshot.value(QStringLiteral("validationState")).toString()!=QStringLiteral("VALIDATED")) {
        error=QStringLiteral("VALIDATED_CANONICAL_ROUTE_REQUIRED_FOR_WEATHER_ASSESSMENT");
        return false;
    }
    if (!snapshot.value(QStringLiteral("trajectoryGeometryComplete")).toBool()) {
        error=QStringLiteral("COMPLETE_TRAJECTORY_GEOMETRY_REQUIRED");
        return false;
    }
    const QVariantList points=snapshot.value(QStringLiteral("waypoints")).toList();
    const QVariantList segments=snapshot.value(QStringLiteral("segments")).toList();
    if (points.size()<2 || segments.size()!=points.size()-1) {
        error=QStringLiteral("COMPLETE_ORDERED_ROUTE_SEGMENTS_REQUIRED");
        return false;
    }
    std::set<std::string> waypointIds;
    for (const auto& value:points) {
        const QVariantMap item=value.toMap();
        const QString id=item.value(QStringLiteral("waypointId")).toString().trimmed();
        bool latOk=false,lonOk=false,altOk=false;
        const double lat=item.value(QStringLiteral("latitudeDeg")).toDouble(&latOk);
        const double lon=item.value(QStringLiteral("longitudeDeg")).toDouble(&lonOk);
        const double altitude=item.value(QStringLiteral("altitudeM")).toDouble(&altOk);
        if (id.isEmpty() || !waypointIds.insert(id.toStdString()).second
            || !latOk || !lonOk || !altOk || !std::isfinite(lat) || !std::isfinite(lon)
            || !std::isfinite(altitude) || lat < -90.0 || lat > 90.0
            || lon < -180.0 || lon > 180.0 || altitude < 0.0
            || item.value(QStringLiteral("altitudeReference")).toString()!=QStringLiteral("AGL")) {
            error=QStringLiteral("ROUTE_WAYPOINT_REQUIRES_UNIQUE_ID_VALID_COORDINATES_AND_AGL");
            return false;
        }
        RouteWaypoint waypoint;
        waypoint.waypoint_id=id.toStdString();
        waypoint.position={lat,lon};
        waypoint.altitude_m=altitude;
        waypoint.altitude_reference=AltitudeReference::AGL;
        waypoint.mandatory=item.value(QStringLiteral("mandatory")).toBool();
        const bool hasTaskDuration=item.contains(QStringLiteral("taskExecutionDurationS"));
        const bool hasTaskEnergy=item.contains(QStringLiteral("taskExecutionEnergyWh"));
        if (hasTaskDuration!=hasTaskEnergy) {
            error=QStringLiteral("TASK_EXECUTION_DURATION_AND_ENERGY_MUST_BE_SUPPLIED_TOGETHER");
            return false;
        }
        if (hasTaskDuration) {
            bool durationOk=false,energyOk=false;
            const double duration=item.value(QStringLiteral("taskExecutionDurationS")).toDouble(&durationOk);
            const double energy=item.value(QStringLiteral("taskExecutionEnergyWh")).toDouble(&energyOk);
            if (!durationOk || !energyOk || !std::isfinite(duration) || !std::isfinite(energy)
                || duration<0.0 || energy<0.0) {
                error=QStringLiteral("TASK_EXECUTION_DURATION_AND_ENERGY_MUST_BE_FINITE_NON_NEGATIVE");
                return false;
            }
            waypoint.task_execution_duration_s=duration;
            waypoint.task_execution_energy_wh=energy;
        }
        route.waypoints.push_back(std::move(waypoint));
    }
    std::set<std::string> segmentIds;
    for (qsizetype i=0;i<segments.size();++i) {
        const QVariantMap item=segments.at(i).toMap();
        const QString id=item.value(QStringLiteral("segmentId")).toString().trimmed();
        const QString from=item.value(QStringLiteral("fromWaypointId")).toString().trimmed();
        const QString to=item.value(QStringLiteral("toWaypointId")).toString().trimmed();
        bool distanceOk=false,speedOk=false;
        const double distance=item.value(QStringLiteral("distanceM")).toDouble(&distanceOk);
        const double speed=item.value(QStringLiteral("nominalSpeedMps")).toDouble(&speedOk);
        if (id.isEmpty() || !segmentIds.insert(id.toStdString()).second
            || from!=QString::fromStdString(route.waypoints[static_cast<std::size_t>(i)].waypoint_id)
            || to!=QString::fromStdString(route.waypoints[static_cast<std::size_t>(i+1)].waypoint_id)
            || !distanceOk || !speedOk || !std::isfinite(distance) || !std::isfinite(speed)
            || distance<=0.0 || speed<=0.0) {
            error=QStringLiteral("ROUTE_SEGMENT_CHAIN_OR_PERFORMANCE_GEOMETRY_INVALID");
            return false;
        }
        RouteSegment segment;
        segment.segment_id=id.toStdString();
        segment.from_waypoint_id=from.toStdString();
        segment.to_waypoint_id=to.toStdString();
        segment.distance_m=distance;
        segment.nominal_speed_mps=speed;
        const auto setOptional=[&item](const QString& key,std::optional<double>& target) {
            if (!item.contains(key)) return true;
            bool ok=false;
            const double v=item.value(key).toDouble(&ok);
            if (!ok || !std::isfinite(v)) return false;
            target=v;
            return true;
        };
        if (!setOptional(QStringLiteral("bankAngleRad"),segment.bank_angle_rad)
            || !setOptional(QStringLiteral("lateralAccelerationMps2"),segment.lateral_acceleration_mps2)) {
            error=QStringLiteral("ROUTE_SEGMENT_DYNAMIC_LIMIT_INVALID");
            return false;
        }
        route.segments.push_back(std::move(segment));
    }
    if (snapshot.contains(QStringLiteral("speedProfile"))) {
        const QVariantMap sourceProfile = snapshot.value(QStringLiteral("speedProfile")).toMap();
        bluesky::planning::RouteSpeedProfile profile;
        profile.vehicle_model_id = sourceProfile.value(QStringLiteral("vehicleModelId")).toString().trimmed().toStdString();
        profile.vehicle_model_version = sourceProfile.value(QStringLiteral("vehicleModelVersion")).toString().trimmed().toStdString();
        profile.configuration_id = sourceProfile.value(QStringLiteral("configurationId")).toString().trimmed().toStdString();
        profile.speed_reference = sourceProfile.value(QStringLiteral("speedReference")).toString().trimmed().toStdString();
        profile.dynamics_source = sourceProfile.value(QStringLiteral("dynamicsSource")).toString().trimmed().toStdString();
        profile.profile_id = sourceProfile.value(QStringLiteral("profileId")).toString().trimmed().toStdString();
        profile.profile_version = sourceProfile.value(QStringLiteral("profileVersion")).toString().trimmed().toStdString();
        profile.wind_snapshot_id = sourceProfile.value(QStringLiteral("windSnapshotId")).toString().trimmed().toStdString();
        profile.wind_snapshot_version = sourceProfile.value(QStringLiteral("windSnapshotVersion")).toString().trimmed().toStdString();
        profile.camera_configuration_id = sourceProfile.value(QStringLiteral("cameraConfigurationId")).toString().trimmed().toStdString();
        const QVariantList profileValues = sourceProfile.value(QStringLiteral("segments")).toList();
        if (profile.vehicle_model_id.empty() || profile.vehicle_model_version.empty()
            || profile.configuration_id.empty() || profile.dynamics_source.empty()
            || profile.profile_id.empty() || profile.profile_version.empty()
            || (profile.speed_reference != "TAS" && profile.speed_reference != "AIR_RELATIVE")
            || profileValues.size() != segments.size() || profileValues.isEmpty()) {
            error = QStringLiteral("ROUTE_SPEED_PROFILE_IDENTITY_OR_COVERAGE_INVALID");
            return false;
        }
        double previousExitSpeed = 0.0;
        for (qsizetype i = 0; i < profileValues.size(); ++i) {
            const QVariantMap item = profileValues.at(i).toMap();
            bluesky::planning::RouteSpeedProfileSegment segment;
            segment.segment_id = item.value(QStringLiteral("segmentId")).toString().trimmed().toStdString();
            struct ProfileField { const char *key; double *target; };
            ProfileField fields[] = {
                {"distanceM", &segment.distance_m},
                {"entrySpeedMps", &segment.entry_speed_mps},
                {"exitSpeedMps", &segment.exit_speed_mps},
                {"maximumReachableSpeedMps", &segment.maximum_reachable_speed_mps},
                {"accelerationDistanceM", &segment.acceleration_distance_m},
                {"decelerationDistanceM", &segment.deceleration_distance_m},
                {"cruiseDistanceM", &segment.cruise_distance_m},
                {"maximumAccelerationMps2", &segment.maximum_acceleration_mps2},
                {"maximumDecelerationMps2", &segment.maximum_deceleration_mps2},
                {"minimumSpeedMps", &segment.minimum_speed_mps},
                {"maximumSpeedMps", &segment.maximum_speed_mps},
                {"appliedMinimumSpeedMps", &segment.applied_minimum_speed_mps},
                {"appliedMaximumSpeedMps", &segment.applied_maximum_speed_mps}
            };
            for (const auto &field : fields) {
                if (!readFinite(item, QString::fromLatin1(field.key), *field.target)) {
                    error = QStringLiteral("ROUTE_SPEED_PROFILE_NUMERIC_FIELD_INVALID: ")
                        + QString::fromLatin1(field.key);
                    return false;
                }
            }
            const auto &routeSegment = route.segments[static_cast<std::size_t>(i)];
            const double phaseDistance = segment.acceleration_distance_m
                + segment.deceleration_distance_m + segment.cruise_distance_m;
            const double tolerance = std::max(0.5, routeSegment.distance_m * 0.01);
            if (segment.segment_id != routeSegment.segment_id
                || segment.distance_m <= 0.0
                || std::abs(segment.distance_m - routeSegment.distance_m) > tolerance
                || segment.entry_speed_mps <= 0.0 || segment.exit_speed_mps <= 0.0
                || segment.maximum_reachable_speed_mps
                    < std::max(segment.entry_speed_mps, segment.exit_speed_mps)
                || segment.maximum_acceleration_mps2 <= 0.0
                || segment.maximum_deceleration_mps2 <= 0.0
                || segment.acceleration_distance_m < 0.0
                || segment.deceleration_distance_m < 0.0
                || segment.cruise_distance_m < 0.0
                || std::abs(phaseDistance - segment.distance_m) > tolerance
                || segment.minimum_speed_mps <= 0.0
                || segment.maximum_speed_mps < segment.minimum_speed_mps
                || segment.applied_minimum_speed_mps < segment.minimum_speed_mps
                || segment.applied_maximum_speed_mps > segment.maximum_speed_mps
                || segment.applied_maximum_speed_mps < segment.applied_minimum_speed_mps
                || segment.entry_speed_mps < segment.applied_minimum_speed_mps - 1e-6
                || segment.exit_speed_mps < segment.applied_minimum_speed_mps - 1e-6
                || segment.maximum_reachable_speed_mps > segment.applied_maximum_speed_mps + 1e-6
                || (i > 0 && std::abs(segment.entry_speed_mps - previousExitSpeed) > 1e-6)) {
                error = QStringLiteral("ROUTE_SPEED_PROFILE_SEGMENT_BINDING_INVALID");
                return false;
            }
            previousExitSpeed = segment.exit_speed_mps;
            profile.segments.push_back(std::move(segment));
        }
        route.speed_profile = std::move(profile);
    }
    route.trajectory_geometry_complete=true;
    return true;
}

bool buildFlightSafetyEnvelope(const QVariantMap& profile,
                               bluesky::planning::FlightSafetyEnvelope& envelope,
                               QString& error)
{
    const QString modelId = profile.value(QStringLiteral("vehicleModelId")).toString().trimmed();
    const QString modelVersion = profile.value(QStringLiteral("vehicleModelVersion")).toString().trimmed();
    const QString configurationId = profile.value(QStringLiteral("configurationId")).toString().trimmed();
    const QString declaredModelId = profile.value(QStringLiteral("flightSafetyVehicleModelId")).toString().trimmed();
    const QString declaredModelVersion = profile.value(QStringLiteral("flightSafetyModelVersion")).toString().trimmed();
    const QString declaredConfigurationId = profile.value(QStringLiteral("flightSafetyConfigurationId")).toString().trimmed();
    const QString declaredEnvelopeId = profile.value(QStringLiteral("flightSafetyEnvelopeId")).toString().trimmed();
    const QString motionClass = profile.value(QStringLiteral("flightSafetyMotionClass")).toString().trimmed();
    const QString evidence = profile.value(QStringLiteral("flightSafetyEvidence")).toString().trimmed();

    if (modelId.isEmpty() || modelVersion.isEmpty() || configurationId.isEmpty()
        || declaredEnvelopeId.isEmpty() || declaredModelId != modelId
        || declaredModelVersion != modelVersion || declaredConfigurationId != configurationId) {
        error = QStringLiteral("FLIGHT_SAFETY_ENVELOPE_MODEL_VERSION_CONFIGURATION_MISMATCH");
        return false;
    }

    envelope.vehicle_model_id = modelId.toStdString();
    envelope.model_version = modelVersion.toStdString();
    envelope.configuration_id = configurationId.toStdString();
    envelope.source_reference = profile.value(QStringLiteral("flightSafetySourceReference")).toString().trimmed().toStdString();
    envelope.speed_reference = profile.value(QStringLiteral("flightSafetySpeedReference")).toString().trimmed().toStdString();
    envelope.minimum_speed_basis = profile.value(QStringLiteral("flightSafetyMinimumSpeedBasis")).toString().trimmed().toStdString();

    if (motionClass == QStringLiteral("FIXED_WING"))
        envelope.motion_class = bluesky::planning::MotionClass::FixedWing;
    else if (motionClass == QStringLiteral("MULTIROTOR"))
        envelope.motion_class = bluesky::planning::MotionClass::Multirotor;
    else {
        error = QStringLiteral("FLIGHT_SAFETY_MOTION_CLASS_UNKNOWN");
        return false;
    }

    if (evidence == QStringLiteral("OFFICIAL_VALIDATED"))
        envelope.evidence = bluesky::planning::FlightSafetyEvidence::OfficialValidated;
    else if (evidence == QStringLiteral("FLIGHT_TEST_VALIDATED"))
        envelope.evidence = bluesky::planning::FlightSafetyEvidence::FlightTestValidated;
    else {
        error = QStringLiteral("FLIGHT_SAFETY_EVIDENCE_NOT_QUALIFIED");
        return false;
    }

    double value = 0.0;
    if (!readFinite(profile, QStringLiteral("flightSafetyMinimumSpeedMps"), value) || value <= 0.0) {
        error = QStringLiteral("FLIGHT_SAFETY_MINIMUM_SPEED_REQUIRED");
        return false;
    }
    envelope.minimum_speed_mps = value;
    if (!readFinite(profile, QStringLiteral("flightSafetyMaximumSpeedMps"), value) || value <= 0.0) {
        error = QStringLiteral("FLIGHT_SAFETY_MAXIMUM_SPEED_REQUIRED");
        return false;
    }
    envelope.maximum_speed_mps = value;

    if (envelope.motion_class == bluesky::planning::MotionClass::FixedWing) {
        if (readFinite(profile, QStringLiteral("flightSafetyMaximumBankAngleRad"), value))
            envelope.maximum_bank_angle_rad = value;
    } else {
        if (readFinite(profile, QStringLiteral("flightSafetyMaximumLateralAccelerationMps2"), value))
            envelope.maximum_lateral_acceleration_mps2 = value;
    }

    if (envelope.source_reference.empty() || envelope.speed_reference.empty()
        || envelope.minimum_speed_basis.empty()) {
        error = QStringLiteral("FLIGHT_SAFETY_PROVENANCE_AND_SPEED_BASIS_REQUIRED");
        return false;
    }
    return true;
}

bool validateVehicleAssignment(const QString& missionJson,const QVariantMap& profile,
                               QString& vehicleId,QString& configurationVersion,
                               QString& missionId,QString& error)
{
    vehicleId=profile.value(QStringLiteral("vehicleId")).toString().trimmed();
    configurationVersion=profile.value(QStringLiteral("configurationVersion")).toString().trimmed();
    const QString validation=profile.value(QStringLiteral("validationState")).toString();
    const QString approval=profile.value(QStringLiteral("approvalState")).toString();
    if (vehicleId.isEmpty() || configurationVersion.isEmpty()
        || profile.value(QStringLiteral("validForMission")).toBool()!=true
        || validation!=QStringLiteral("VALIDATED") || approval!=QStringLiteral("APPROVED")) {
        error=QStringLiteral("VALIDATED_APPROVED_VEHICLE_PERFORMANCE_PROFILE_REQUIRED");
        return false;
    }
    QJsonParseError parseError;
    const auto document=QJsonDocument::fromJson(missionJson.toUtf8(),&parseError);
    if (parseError.error!=QJsonParseError::NoError || !document.isObject()) {
        error=QStringLiteral("CANONICAL_MISSION_JSON_INVALID");
        return false;
    }
    const QJsonObject mission=document.object();
    missionId=jsonString(mission,"missionId","mission_id");
    if (missionId.isEmpty()) {
        const QJsonObject identity=mission.value(QStringLiteral("identity")).toObject();
        missionId=jsonString(identity,"missionId","mission_id");
    }
    if (missionId.isEmpty()) {
        error=QStringLiteral("CANONICAL_MISSION_ID_REQUIRED_FOR_WEATHER_ASSESSMENT");
        return false;
    }
    QJsonArray assignments=mission.value(QStringLiteral("vehicleAssignments")).toArray();
    if (assignments.isEmpty())
        assignments=mission.value(QStringLiteral("vehicle_assignments")).toArray();
    if (assignments.isEmpty()) {
        error=QStringLiteral("MISSION_HAS_NO_APPROVED_VEHICLE_ASSIGNMENT");
        return false;
    }
    bool matched=false;
    for (const auto& value:assignments) {
        const QJsonObject assignment=value.toObject();
        const QString assignedVehicle=jsonString(assignment,"vehicleId","vehicle_id");
        const QString assignedConfiguration=jsonString(assignment,"configurationVersion","configuration_version");
        const QString assignedValidation=jsonString(assignment,"validationState","validation_state");
        const QString assignedApproval=jsonString(assignment,"approvalState","approval_state");
        if (assignedVehicle==vehicleId && assignedConfiguration==configurationVersion
            && assignedValidation==QStringLiteral("VALIDATED")
            && assignedApproval==QStringLiteral("APPROVED")) {
            matched=true;
            break;
        }
    }
    if (!matched) {
        error=QStringLiteral("VEHICLE_PROFILE_DOES_NOT_MATCH_APPROVED_MISSION_ASSIGNMENT");
        return false;
    }
    return true;
}

bool buildPerformanceInput(const QVariantMap& profile,const Route& route,
                           const QString& missionStartUtc,
                           TrajectoryPerformanceInput& input,QString& error)
{
    input.uav_id=profile.value(QStringLiteral("vehicleId")).toString().toStdString();
    input.configuration_version=profile.value(QStringLiteral("configurationVersion")).toString().toStdString();
    input.performance_version=profile.value(QStringLiteral("performanceVersion")).toString().toStdString();
    input.mission_start_utc=missionStartUtc.toStdString();
    if (input.performance_version.empty()) {
        error=QStringLiteral("VEHICLE_PERFORMANCE_MODEL_VERSION_REQUIRED");
        return false;
    }
    struct NumericField { const char* key; double* target; };
    const NumericField fields[]={
        {"cruiseAirspeedMps",&input.cruise_airspeed_mps},
        {"climbRateMps",&input.climb_rate_mps},
        {"descentRateMps",&input.descent_rate_mps},
        {"windToleranceMps",&input.wind_tolerance_mps},
        {"energyPerHorizontalMeterWh",&input.energy_per_horizontal_meter_wh},
        {"energyPerClimbMeterWh",&input.energy_per_climb_meter_wh},
        {"energyPerDescentMeterWh",&input.energy_per_descent_meter_wh},
        {"usableEnergyWh",&input.usable_energy_wh},
        {"reserveRequirementWh",&input.reserve_requirement_wh}
    };
    for (const auto& field:fields) {
        double value=0.0;
        if (!readFinite(profile,QString::fromLatin1(field.key),value)) {
            error=QStringLiteral("VEHICLE_PERFORMANCE_FIELD_REQUIRED: ")+QString::fromLatin1(field.key);
            return false;
        }
        *field.target=value;
    }
    if (route.speed_profile) {
        const auto &speedProfile = *route.speed_profile;
        const QString declaredModelId = profile.value(QStringLiteral("vehicleModelId")).toString().trimmed();
        const QString declaredModelVersion = profile.value(QStringLiteral("vehicleModelVersion")).toString().trimmed();
        const QString declaredConfigurationId = profile.value(QStringLiteral("configurationId")).toString().trimmed();
        if (speedProfile.vehicle_model_id != declaredModelId.toStdString()
            || speedProfile.vehicle_model_version != declaredModelVersion.toStdString()
            || speedProfile.configuration_id != declaredConfigurationId.toStdString()
            || speedProfile.profile_id.empty() || speedProfile.profile_version.empty()
            || speedProfile.segments.size() != route.segments.size()) {
            error = QStringLiteral("ROUTE_SPEED_PROFILE_VEHICLE_OR_REVISION_BINDING_MISMATCH");
            return false;
        }
        input.speed_profile_id = speedProfile.profile_id;
        input.speed_profile_version = speedProfile.profile_version;
        input.speed_profile_vehicle_model_id = speedProfile.vehicle_model_id;
        input.speed_profile_vehicle_model_version = speedProfile.vehicle_model_version;
        input.speed_profile_configuration_id = speedProfile.configuration_id;
        input.speed_profile_speed_reference = speedProfile.speed_reference;
        input.speed_profile_segments.reserve(speedProfile.segments.size());
        for (const auto &segment : speedProfile.segments) {
            input.speed_profile_segments.push_back({
                segment.segment_id, segment.distance_m, segment.entry_speed_mps,
                segment.exit_speed_mps, segment.maximum_reachable_speed_mps,
                segment.acceleration_distance_m, segment.deceleration_distance_m,
                segment.cruise_distance_m, segment.maximum_acceleration_mps2,
                segment.maximum_deceleration_mps2
            });
        }
    }
    if (input.cruise_airspeed_mps<=0.0 || input.climb_rate_mps<=0.0
        || input.descent_rate_mps<=0.0 || input.wind_tolerance_mps<=0.0
        || input.energy_per_horizontal_meter_wh<=0.0
        || input.energy_per_climb_meter_wh<=0.0 || input.energy_per_descent_meter_wh<=0.0
        || input.usable_energy_wh<=input.reserve_requirement_wh
        || input.reserve_requirement_wh<0.0) {
        error=QStringLiteral("VEHICLE_PERFORMANCE_LIMITS_INVALID");
        return false;
    }
    return true;
}

struct ForecastProduct {
    QString path;
    QString sha256;
    qint64 bytes{0};
    int forecastHour{-1};
    QString modelRunUtc;
};

bool validateForecastProducts(const QVariantList& values,
                              std::vector<ForecastProduct>& products,
                              QString& error)
{
    if (values.isEmpty()) {
        error=QStringLiteral("DOWNLOADED_GRIB2_PRODUCTS_REQUIRED");
        return false;
    }
    QSet<int> forecastHours;
    QString commonModelRun;
    for (const QVariant& value:values) {
        const QVariantMap item=value.toMap();
        const QString path=item.value(QStringLiteral("filePath")).toString();
        const QVariantMap manifest=item.value(QStringLiteral("manifest")).toMap();
        bool forecastHourOk=false,schemaOk=false,manifestHourOk=false;
        const int forecastHour=item.value(QStringLiteral("forecastHour")).toInt(&forecastHourOk);
        const int schema=manifest.value(QStringLiteral("cache_schema_version")).toInt(&schemaOk);
        const int manifestHour=manifest.value(QStringLiteral("forecast_hour")).toInt(&manifestHourOk);
        const QString provider=manifest.value(QStringLiteral("provider")).toString();
        const QString model=manifest.value(QStringLiteral("model")).toString();
        const QString product=manifest.value(QStringLiteral("product")).toString();
        const QString modelRunUtc=manifest.value(QStringLiteral("model_run_utc")).toString();
        const QString validUtc=manifest.value(QStringLiteral("forecast_valid_time_utc")).toString();
        const QString sha=manifest.value(QStringLiteral("sha256")).toString().toLower();
        const QString fileName=manifest.value(QStringLiteral("file")).toString();
        const QString decodeStatus=manifest.value(QStringLiteral("decode_status")).toString();
        const QString optimizerStatus=manifest.value(QStringLiteral("optimizer_use_status")).toString();
        bool bytesOk=false;
        const qint64 bytes=manifest.value(QStringLiteral("bytes")).toLongLong(&bytesOk);
        const QDateTime modelRun=QDateTime::fromString(modelRunUtc,Qt::ISODate).toUTC();
        const QDateTime validTime=QDateTime::fromString(validUtc,Qt::ISODate).toUTC();
        const QDateTime expectedValid=modelRun.addSecs(qint64(forecastHour)*3600);
        const QVariantList variables=manifest.value(QStringLiteral("variables")).toList();
        if (path.isEmpty() || !QFileInfo(path).isFile() || schema!=1
            || provider!=QStringLiteral("NOAA/NCEP NOMADS")
            || model!=QStringLiteral("GFS 0.25 degree")
            || product!=QStringLiteral("GRIB2 regional wind subset")
            || modelRunUtc.isEmpty() || !modelRun.isValid() || !validTime.isValid()
            || !forecastHourOk || !schemaOk || !manifestHourOk
            || forecastHour<0 || forecastHour!=manifestHour
            || validTime!=expectedValid || sha.size()!=64 || !bytesOk || bytes<=0
            || QFileInfo(path).size()!=bytes || fileName!=QFileInfo(path).fileName()
            || decodeStatus!=QStringLiteral("NOT_PERFORMED")
            || optimizerStatus!=QStringLiteral("BLOCKED_UNTIL_DECODED_AND_QC_PASSED")
            || manifest.value(QStringLiteral("forecast_time_semantics")).toString()
                !=QStringLiteral("VALID_TIME_INSTANT")
            || manifest.value(QStringLiteral("classification")).toString()
                !=QStringLiteral("LIVE_MODEL_DATA_UNDECODED")
            || manifest.value(QStringLiteral("request_url")).toString().isEmpty()
            || manifest.value(QStringLiteral("levels_m_agl")).toList().isEmpty()
            || !variables.contains(QStringLiteral("UGRD"))
            || !variables.contains(QStringLiteral("VGRD"))) {
            error=QStringLiteral("GRIB2_PRODUCT_MANIFEST_OR_FILE_IDENTITY_INVALID: ")+path;
            return false;
        }
        if (!commonModelRun.isEmpty() && commonModelRun!=modelRunUtc) {
            error=QStringLiteral("GRIB2_PRODUCTS_MUST_USE_ONE_GFS_MODEL_RUN");
            return false;
        }
        if (forecastHours.contains(forecastHour)) {
            error=QStringLiteral("DUPLICATE_GFS_FORECAST_LEAD_HOUR");
            return false;
        }
        commonModelRun=modelRunUtc;
        forecastHours.insert(forecastHour);
        products.push_back({path,sha,bytes,forecastHour,modelRunUtc});
    }
    std::sort(products.begin(),products.end(),
        [](const ForecastProduct& a,const ForecastProduct& b) {
            return a.forecastHour<b.forecastHour;
        });
    return true;
}

QVariantMap evaluateOnWorker(const std::vector<ForecastProduct>& products,const Route& route,
                             const TrajectoryPerformanceInput& input)
{
    QVariantMap result;
#ifndef BLUESKY_HAVE_ECCODES
    Q_UNUSED(products);
    Q_UNUSED(route);
    Q_UNUSED(input);
    result.insert(QStringLiteral("weatherReady"),false);
    result.insert(QStringLiteral("decodeSucceeded"),false);
    result.insert(QStringLiteral("weatherError"),QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
    return result;
#else
    QCryptographicHash snapshotHash(QCryptographicHash::Sha256);
    std::vector<std::string> paths;
    paths.reserve(products.size());
    for (const auto& product:products) {
        QFile file(product.path);
        if (!file.open(QIODevice::ReadOnly)) {
            result.insert(QStringLiteral("weatherReady"),false);
            result.insert(QStringLiteral("decodeSucceeded"),false);
            result.insert(QStringLiteral("weatherError"),
                          QStringLiteral("FORECAST_PRODUCT_UNREADABLE: ")+product.path);
            return result;
        }
        QCryptographicHash fileHash(QCryptographicHash::Sha256);
        qint64 actualBytes=0;
        while (!file.atEnd()) {
            const QByteArray chunk=file.read(1024*1024);
            if (chunk.isEmpty() && file.error()!=QFileDevice::NoError) {
                result.insert(QStringLiteral("weatherReady"),false);
                result.insert(QStringLiteral("decodeSucceeded"),false);
                result.insert(QStringLiteral("weatherError"),
                              QStringLiteral("FORECAST_PRODUCT_READ_FAILED: ")+product.path);
                return result;
            }
            actualBytes+=chunk.size();
            fileHash.addData(chunk);
        }
        const QString actualSha=QString::fromLatin1(fileHash.result().toHex());
        if (actualBytes!=product.bytes || actualSha!=product.sha256) {
            result.insert(QStringLiteral("weatherReady"),false);
            result.insert(QStringLiteral("decodeSucceeded"),false);
            result.insert(QStringLiteral("weatherError"),
                          QStringLiteral("GRIB2_PRODUCT_SHA256_OR_SIZE_MISMATCH: ")+product.path);
            return result;
        }
        snapshotHash.addData(actualSha.toLatin1());
        snapshotHash.addData(QByteArray(1,'\0'));
        paths.push_back(QFile::encodeName(product.path).toStdString());
    }
    const QString snapshotId=QStringLiteral("GFS-WIND-")+QString::fromLatin1(snapshotHash.result().toHex());
    const auto evaluation=bluesky::weather::Grib2WeatherPipeline::evaluateFiles(
        paths,route,snapshotId.toStdString(),"NOAA-GFS-0.25-GRIB2",
        input,"WEATHER-TIME-DEPENDENT-ITERATIVE-V1");
    result.insert(QStringLiteral("decodeSucceeded"),evaluation.decode_succeeded);
    result.insert(QStringLiteral("weatherReady"),evaluation.evaluation.weather_ready);
    result.insert(QStringLiteral("weatherError"),QString::fromStdString(evaluation.evaluation.weather_error));
    result.insert(QStringLiteral("snapshotId"),snapshotId);
    result.insert(QStringLiteral("modelRunUtc"),products.front().modelRunUtc);
    result.insert(QStringLiteral("forecastProductCount"),static_cast<int>(products.size()));
    if (!evaluation.evaluation.weather_ready) return result;

    const auto& trajectory=evaluation.evaluation.trajectory;
    result.insert(QStringLiteral("trajectoryStatus"),
                  trajectory.status==bluesky::planning::TrajectoryStatus::Feasible
                    ? QStringLiteral("FEASIBLE") : QStringLiteral("INFEASIBLE"));
    result.insert(QStringLiteral("routeId"),QString::fromStdString(trajectory.route_id));
    result.insert(QStringLiteral("routeVersion"),QString::fromStdString(trajectory.route_version));
    result.insert(QStringLiteral("uavId"),QString::fromStdString(trajectory.uav_id));
    result.insert(QStringLiteral("configurationVersion"),QString::fromStdString(trajectory.configuration_version));
    result.insert(QStringLiteral("dependencyIdentity"),QString::fromStdString(trajectory.dependency_identity));
    result.insert(QStringLiteral("totalTimeS"),trajectory.total_time_s);
    result.insert(QStringLiteral("totalEnergyWh"),trajectory.total_energy_wh);
    result.insert(QStringLiteral("remainingEnergyWh"),trajectory.remaining_energy_wh);
    QVariantList segments;
    for (const auto& segment:trajectory.segments) {
        segments.push_back(QVariantMap{
            {QStringLiteral("segmentId"),QString::fromStdString(segment.segment_id)},
            {QStringLiteral("groundSpeedMps"),segment.ground_speed_mps},
            {QStringLiteral("trackDeg"),segment.track_deg},
            {QStringLiteral("traversalTimeS"),segment.traversal_time_s},
            {QStringLiteral("energyWh"),segment.energy_wh}
        });
    }
    result.insert(QStringLiteral("segments"),segments);
    QVariantList findings;
    for (const auto& finding:trajectory.findings) {
        findings.push_back(QVariantMap{
            {QStringLiteral("segmentId"),QString::fromStdString(finding.segment_id)},
            {QStringLiteral("detail"),QString::fromStdString(finding.detail)}
        });
    }
    result.insert(QStringLiteral("findings"),findings);
    return result;
#endif
}

class CandidateSetSolver final : public bluesky::planning::Solver
{
public:
    explicit CandidateSetSolver(std::vector<bluesky::planning::CandidateSolution> candidates)
        : m_candidates(std::move(candidates)) {}

    bluesky::planning::SolverMetadata metadata() const override
    {
        return {"hmi-weather-candidate-set", "1", {"point_to_point"}, false, true};
    }

    bool eligible(const bluesky::planning::MissionProblem& problem) const override
    {
        return problem.problem_class == "point_to_point";
    }

    bluesky::planning::RunState run(bluesky::planning::SolverContext& context) override
    {
        for (const auto& candidate : m_candidates) {
            if (context.cancelled()) return bluesky::planning::RunState::Cancelled;
            context.publish(candidate);
        }
        return bluesky::planning::RunState::Completed;
    }

    void cancel() override {}

private:
    std::vector<bluesky::planning::CandidateSolution> m_candidates;
};

class CandidateSetContext final : public bluesky::planning::SolverContext
{
public:
    explicit CandidateSetContext(const bluesky::planning::MissionProblem& problem)
        : m_problem(problem) {}

    const bluesky::planning::MissionProblem& problem() const override { return m_problem; }
    const bluesky::planning::ComputeBudget& budget() const override { return m_budget; }
    bool cancelled() const override { return false; }
    void publish(bluesky::planning::CandidateSolution candidate) override
    {
        m_candidates.push_back(std::move(candidate));
    }
    const std::vector<bluesky::planning::CandidateSolution>& candidates() const override
    {
        return m_candidates;
    }

private:
    const bluesky::planning::MissionProblem& m_problem;
    bluesky::planning::ComputeBudget m_budget{5000, 512, 64};
    std::vector<bluesky::planning::CandidateSolution> m_candidates;
};


bool parseNormalizedNotamSnapshot(const QVariantMap& value,
                                  bluesky::planning::NotamSnapshot& snapshot,
                                  double& safetyBufferM,
                                  QString& error)
{
    using namespace bluesky::planning;
    if (value.value(QStringLiteral("valid")).toBool() != true
        || !value.contains(QStringLiteral("zones"))) {
        error = QStringLiteral("NOTAM_SNAPSHOT_REQUIRED_FOR_WEATHER_RERANKING");
        return false;
    }
    const auto requiredString = [&value](const QString& key) {
        return value.value(key).toString().trimmed();
    };
    snapshot.snapshot_id = requiredString(QStringLiteral("snapshotId")).toStdString();
    snapshot.source_id = requiredString(QStringLiteral("sourceId")).toStdString();
    snapshot.source_version = requiredString(QStringLiteral("sourceVersion")).toStdString();
    bool epochOk = false, latOk = false, lonOk = false, radiusOk = false;
    snapshot.evaluation_epoch_s = value.value(QStringLiteral("evaluationEpochS")).toLongLong(&epochOk);
    snapshot.coverage_center.latitude_deg = value.value(QStringLiteral("coverageCenterLatitudeDeg")).toDouble(&latOk);
    snapshot.coverage_center.longitude_deg = value.value(QStringLiteral("coverageCenterLongitudeDeg")).toDouble(&lonOk);
    snapshot.coverage_radius_m = value.value(QStringLiteral("coverageRadiusM")).toDouble(&radiusOk);
    bool startOk = false, endOk = false, bufferOk = false;
    const qint64 missionStart = value.value(QStringLiteral("plannedMissionStartEpochS")).toLongLong(&startOk);
    const qint64 missionEnd = value.value(QStringLiteral("plannedMissionEndEpochS")).toLongLong(&endOk);
    snapshot.planned_mission_start_epoch_s = missionStart;
    snapshot.planned_mission_end_epoch_s = missionEnd;
    safetyBufferM = value.value(QStringLiteral("safetyBufferM")).toDouble(&bufferOk);
    if (snapshot.snapshot_id.empty() || snapshot.source_id.empty() || snapshot.source_version.empty()
        || !epochOk || snapshot.evaluation_epoch_s <= 0
        || !latOk || !lonOk || !radiusOk
        || !std::isfinite(snapshot.coverage_center.latitude_deg)
        || !std::isfinite(snapshot.coverage_center.longitude_deg)
        || snapshot.coverage_center.latitude_deg < -90.0 || snapshot.coverage_center.latitude_deg > 90.0
        || snapshot.coverage_center.longitude_deg < -180.0 || snapshot.coverage_center.longitude_deg > 180.0
        || !std::isfinite(snapshot.coverage_radius_m) || snapshot.coverage_radius_m <= 0.0
        || !startOk || !endOk || missionStart <= 0 || missionEnd < missionStart
        || !bufferOk || !std::isfinite(safetyBufferM) || safetyBufferM <= 0.0) {
        error = QStringLiteral("NOTAM_SNAPSHOT_IDENTITY_COVERAGE_OR_TIME_WINDOW_INVALID");
        return false;
    }
    for (const QVariant& zoneValue : value.value(QStringLiteral("zones")).toList()) {
        const QVariantMap item = zoneValue.toMap();
        NotamZone zone;
        zone.notam_id = item.value(QStringLiteral("notamId")).toString().trimmed().toStdString();
        zone.component_id = item.value(QStringLiteral("componentId")).toString().trimmed().toStdString();
        if (zone.notam_id.empty() || zone.component_id.empty()
            || item.value(QStringLiteral("geometryType")).toString() != QStringLiteral("POLYGON")) {
            error = QStringLiteral("NOTAM_NORMALIZED_COMPONENT_ID_OR_GEOMETRY_INVALID");
            return false;
        }
        bool lowerOk = false, upperOk = false, fromOk = false, toOk = false;
        zone.lower_altitude_m = item.value(QStringLiteral("lowerAltitudeM")).toDouble(&lowerOk);
        zone.upper_altitude_m = item.value(QStringLiteral("upperAltitudeM")).toDouble(&upperOk);
        zone.valid_from_epoch_s = item.value(QStringLiteral("validFromEpochS")).toLongLong(&fromOk);
        zone.valid_to_epoch_s = item.value(QStringLiteral("validToEpochS")).toLongLong(&toOk);
        const QString datum = item.value(QStringLiteral("altitudeReference")).toString();
        if (datum == QStringLiteral("AGL")) zone.altitude_reference = NotamAltitudeReference::AGL;
        else if (datum == QStringLiteral("AMSL")) zone.altitude_reference = NotamAltitudeReference::AMSL;
        else if (datum == QStringLiteral("FL")) zone.altitude_reference = NotamAltitudeReference::FlightLevel;
        else zone.altitude_reference = NotamAltitudeReference::Unknown;
        if (!lowerOk || !upperOk || !fromOk || !toOk
            || !std::isfinite(zone.lower_altitude_m) || !std::isfinite(zone.upper_altitude_m)
            || zone.lower_altitude_m > zone.upper_altitude_m
            || zone.valid_from_epoch_s <= 0 || zone.valid_to_epoch_s < zone.valid_from_epoch_s) {
            error = QStringLiteral("NOTAM_NORMALIZED_COMPONENT_VERTICAL_OR_TIME_BOUNDS_INVALID");
            return false;
        }
        const QVariantList points = item.value(QStringLiteral("polygon")).toList();
        if (points.size() < 3) {
            error = QStringLiteral("NOTAM_NORMALIZED_COMPONENT_POLYGON_INVALID");
            return false;
        }
        for (const QVariant& pointValue : points) {
            const QVariantMap point = pointValue.toMap();
            bool pointLatOk = false, pointLonOk = false;
            const double latitude = point.value(QStringLiteral("latitudeDeg")).toDouble(&pointLatOk);
            const double longitude = point.value(QStringLiteral("longitudeDeg")).toDouble(&pointLonOk);
            if (!pointLatOk || !pointLonOk || !std::isfinite(latitude) || !std::isfinite(longitude)
                || latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
                error = QStringLiteral("NOTAM_NORMALIZED_COMPONENT_COORDINATE_INVALID");
                return false;
            }
            zone.polygon.push_back({latitude, longitude});
        }
        zone.geometry_type = NotamGeometryType::Polygon;
        zone.restriction_type = NotamRestrictionType::Prohibited;
        snapshot.zones.push_back(std::move(zone));
    }
    error.clear();
    return true;
}

bool routeWithinNotamQueryCoverage(const bluesky::planning::Route& route,
                                   const bluesky::planning::NotamSnapshot& snapshot,
                                   double safetyBufferM)
{
    constexpr double earthRadiusM = 6371008.8;
    constexpr double pi = 3.14159265358979323846;
    const auto radians = [pi](double value) { return value * pi / 180.0; };
    for (const auto& waypoint : route.waypoints) {
        const double lat1 = radians(snapshot.coverage_center.latitude_deg);
        const double lat2 = radians(waypoint.position.latitude_deg);
        const double dLat = lat2 - lat1;
        double dLonDegrees = waypoint.position.longitude_deg - snapshot.coverage_center.longitude_deg;
        while (dLonDegrees > 180.0) dLonDegrees -= 360.0;
        while (dLonDegrees < -180.0) dLonDegrees += 360.0;
        const double dLon = radians(dLonDegrees);
        const double sinLat = std::sin(dLat * 0.5);
        const double sinLon = std::sin(dLon * 0.5);
        const double h = sinLat * sinLat + std::cos(lat1) * std::cos(lat2) * sinLon * sinLon;
        const double distanceM = 2.0 * earthRadiusM * std::asin(std::sqrt(std::clamp(h, 0.0, 1.0)));
        if (!std::isfinite(distanceM) || distanceM + safetyBufferM > snapshot.coverage_radius_m + 0.01)
            return false;
    }
    return !route.waypoints.empty();
}

bool missionCoordinateWithinNotamCoverage(double latitude, double longitude,
                                          const bluesky::planning::NotamSnapshot& snapshot,
                                          double safetyBufferM)
{
    if (!std::isfinite(latitude) || !std::isfinite(longitude)
        || latitude < -90.0 || latitude > 90.0
        || longitude < -180.0 || longitude > 180.0)
        return false;
    constexpr double earthRadiusM = 6371008.8;
    constexpr double pi = 3.14159265358979323846;
    const double lat1 = snapshot.coverage_center.latitude_deg * pi / 180.0;
    const double lat2 = latitude * pi / 180.0;
    const double dLat = lat2 - lat1;
    double dLonDegrees = longitude - snapshot.coverage_center.longitude_deg;
    while (dLonDegrees > 180.0) dLonDegrees -= 360.0;
    while (dLonDegrees < -180.0) dLonDegrees += 360.0;
    const double dLon = dLonDegrees * pi / 180.0;
    const double sinLat = std::sin(dLat * 0.5);
    const double sinLon = std::sin(dLon * 0.5);
    const double h = sinLat * sinLat + std::cos(lat1) * std::cos(lat2) * sinLon * sinLon;
    const double distanceM = 2.0 * earthRadiusM * std::asin(std::sqrt(std::clamp(h, 0.0, 1.0)));
    return std::isfinite(distanceM)
        && distanceM + safetyBufferM <= snapshot.coverage_radius_m + 0.01;
}

bool inspectMissionGeometryCoordinates(const QJsonValue& value,
                                       const bluesky::planning::NotamSnapshot& snapshot,
                                       double safetyBufferM,
                                       bool& foundCoordinate)
{
    if (value.isArray()) {
        const QJsonArray array = value.toArray();
        if (array.size() >= 2 && array.at(0).isDouble() && array.at(1).isDouble()) {
            foundCoordinate = true;
            return missionCoordinateWithinNotamCoverage(array.at(1).toDouble(),
                                                         array.at(0).toDouble(),
                                                         snapshot, safetyBufferM);
        }
        for (const QJsonValue& child : array) {
            if (!inspectMissionGeometryCoordinates(child, snapshot, safetyBufferM, foundCoordinate))
                return false;
        }
    } else if (value.isObject()) {
        const QJsonObject object = value.toObject();
        if (object.contains(QStringLiteral("coordinates")))
            return inspectMissionGeometryCoordinates(object.value(QStringLiteral("coordinates")),
                                                      snapshot, safetyBufferM, foundCoordinate);
        if (object.contains(QStringLiteral("geometry")))
            return inspectMissionGeometryCoordinates(object.value(QStringLiteral("geometry")),
                                                      snapshot, safetyBufferM, foundCoordinate);
    }
    return true;
}

bool missionGeometryWithinNotamQueryCoverage(const QString& missionModelJson,
                                             const bluesky::planning::NotamSnapshot& snapshot,
                                             double safetyBufferM)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(missionModelJson.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
        return false;
    const QJsonValue geometryValue = document.object().value(QStringLiteral("geometryObjects"));
    if (!geometryValue.isArray())
        return false;
    // Point-to-point missions may have no area geometry; their complete route
    // is checked independently by routeWithinNotamQueryCoverage().
    if (geometryValue.toArray().isEmpty())
        return true;
    bool foundCoordinate = false;
    for (const QJsonValue& geometry : geometryValue.toArray()) {
        if (!inspectMissionGeometryCoordinates(geometry, snapshot, safetyBufferM, foundCoordinate))
            return false;
    }
    return foundCoordinate;
}

WeatherCalculationWorkerResult evaluateCandidateSetOnWorker(
    const std::vector<ForecastProduct>& products,
    const std::vector<bluesky::planning::CandidateSolution>& candidates,
    const bluesky::planning::MissionProblem& problem,
    const std::map<std::string, TrajectoryPerformanceInput>& performanceByCandidate,
    const bluesky::planning::NotamSnapshot& notamSnapshot)
{
    WeatherCalculationWorkerResult worker;
    QVariantMap& result = worker.presentation;
#ifndef BLUESKY_HAVE_ECCODES
    Q_UNUSED(products);
    Q_UNUSED(candidates);
    Q_UNUSED(problem);
    Q_UNUSED(performanceByCandidate);
    result.insert(QStringLiteral("weatherReady"), false);
    result.insert(QStringLiteral("decodeSucceeded"), false);
    result.insert(QStringLiteral("weatherError"), QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
    result.insert(QStringLiteral("flightReleaseEligible"), false);
    return worker;
#else
    QCryptographicHash snapshotHash(QCryptographicHash::Sha256);
    std::vector<std::string> paths;
    paths.reserve(products.size());
    for (const auto& product : products) {
        QFile file(product.path);
        if (!file.open(QIODevice::ReadOnly)) {
            result.insert(QStringLiteral("weatherReady"), false);
            result.insert(QStringLiteral("decodeSucceeded"), false);
            result.insert(QStringLiteral("weatherError"),
                          QStringLiteral("FORECAST_PRODUCT_UNREADABLE: ") + product.path);
            result.insert(QStringLiteral("flightReleaseEligible"), false);
            return worker;
        }
        QCryptographicHash fileHash(QCryptographicHash::Sha256);
        qint64 actualBytes = 0;
        while (!file.atEnd()) {
            const QByteArray chunk = file.read(1024 * 1024);
            if (chunk.isEmpty() && file.error() != QFileDevice::NoError) {
                result.insert(QStringLiteral("weatherReady"), false);
                result.insert(QStringLiteral("decodeSucceeded"), false);
                result.insert(QStringLiteral("weatherError"),
                              QStringLiteral("FORECAST_PRODUCT_READ_FAILED: ") + product.path);
                result.insert(QStringLiteral("flightReleaseEligible"), false);
                return worker;
            }
            actualBytes += chunk.size();
            fileHash.addData(chunk);
        }
        const QString actualSha = QString::fromLatin1(fileHash.result().toHex());
        if (actualBytes != product.bytes || actualSha != product.sha256) {
            result.insert(QStringLiteral("weatherReady"), false);
            result.insert(QStringLiteral("decodeSucceeded"), false);
            result.insert(QStringLiteral("weatherError"),
                          QStringLiteral("GRIB2_PRODUCT_SHA256_OR_SIZE_MISMATCH: ") + product.path);
            result.insert(QStringLiteral("flightReleaseEligible"), false);
            return worker;
        }
        snapshotHash.addData(actualSha.toLatin1());
        snapshotHash.addData(QByteArray(1, '\0'));
        paths.push_back(QFile::encodeName(product.path).toStdString());
    }

    const QString snapshotId =
        QStringLiteral("GFS-WIND-") + QString::fromLatin1(snapshotHash.result().toHex());
    const auto snapshot = bluesky::weather::Grib2WeatherPipeline::decodeFiles(
        paths, snapshotId.toStdString(), "NOAA-GFS-0.25-GRIB2");
    result.insert(QStringLiteral("decodeSucceeded"), snapshot.decode_succeeded);
    result.insert(QStringLiteral("weatherSnapshotId"), QString::fromStdString(snapshot.snapshot_id));
    result.insert(QStringLiteral("weatherSnapshotVersion"), QString::fromStdString(snapshot.snapshot_version));
    result.insert(QStringLiteral("modelRunUtc"), products.front().modelRunUtc);
    result.insert(QStringLiteral("forecastProductCount"), static_cast<int>(products.size()));
    result.insert(QStringLiteral("candidateCount"), static_cast<int>(candidates.size()));
    result.insert(QStringLiteral("flightReleaseEligible"), false);
    result.insert(QStringLiteral("assessmentScope"), QStringLiteral("WEATHER_RERANKING_WITH_POST_WIND_NOTAM_GATE"));
    result.insert(QStringLiteral("notamValidationRequired"), true);
    result.insert(QStringLiteral("notamSnapshotId"), QString::fromStdString(notamSnapshot.snapshot_id));
    if (!snapshot.decode_succeeded || snapshot.fields.empty()) {
        result.insert(QStringLiteral("weatherReady"), false);
        result.insert(QStringLiteral("weatherError"),
                      QString::fromStdString(snapshot.error.empty()
                          ? std::string("DECODED_WIND_SNAPSHOT_EMPTY") : snapshot.error));
        return worker;
    }

    auto baseAssessor = bluesky::weather::makeCandidateWeatherAssessor(
        snapshot.fields, performanceByCandidate, snapshot.snapshot_id, snapshot.snapshot_version,
        "WEATHER-TIME-DEPENDENT-ITERATIVE-V1");
    std::map<std::string, bluesky::planning::CandidateWeatherAssessment> assessments;
    bluesky::planning::CandidateWeatherAssessor cachedAssessor =
        [&baseAssessor, &assessments](const bluesky::planning::MissionProblem& mission,
                                     const bluesky::planning::CandidateSolution& candidate) {
            auto assessment = baseAssessor(mission, candidate);
            assessments[candidate.candidate_id] = assessment;
            return assessment;
        };

    std::vector<std::unique_ptr<bluesky::planning::Solver>> solvers;
    solvers.push_back(std::make_unique<CandidateSetSolver>(candidates));
    CandidateSetContext context(problem);
    bluesky::planning::AlgorithmOrchestrator orchestrator(
        std::move(solvers), {}, std::move(cachedAssessor));
    const auto decision = orchestrator.solve(context);

    QVariantList candidateResults;
    for (const auto& candidate : candidates) {
        QVariantMap item;
        item.insert(QStringLiteral("candidateId"), QString::fromStdString(candidate.candidate_id));
        const auto found = assessments.find(candidate.candidate_id);
        if (found == assessments.end()) {
            item.insert(QStringLiteral("weatherEvaluated"), false);
            item.insert(QStringLiteral("weatherError"),
                        QStringLiteral("CANDIDATE_NOT_ASSESSED_BY_ORCHESTRATOR"));
        } else {
            const auto& assessment = found->second;
            item.insert(QStringLiteral("weatherEvaluated"), assessment.evaluated);
            item.insert(QStringLiteral("weatherError"), QString::fromStdString(assessment.error));
            if (assessment.evaluated) {
                item.insert(QStringLiteral("correctedTimeS"), assessment.corrected_time_s);
                item.insert(QStringLiteral("correctedEnergyWh"), assessment.corrected_energy_wh);
                item.insert(QStringLiteral("remainingEnergyWh"), assessment.remaining_energy_wh);
            }
        }
        candidateResults.push_back(item);
    }
    result.insert(QStringLiteral("candidateAssessments"), candidateResults);
    QVariantList admissibleCandidates;
    for (const auto& candidate : decision.admissible_candidates) {
        admissibleCandidates.push_back(QVariantMap{
            {QStringLiteral("candidateId"), QString::fromStdString(candidate.candidate_id)},
            {QStringLiteral("estimatedTimeS"), candidate.estimated_time_s},
            {QStringLiteral("estimatedEnergyWh"), candidate.estimated_energy_wh},
            {QStringLiteral("estimatedReserveWh"), candidate.estimated_reserve_wh},
            {QStringLiteral("objectiveScore"), candidate.objective_score}
        });
    }
    result.insert(QStringLiteral("admissibleCandidates"), admissibleCandidates);
    result.insert(QStringLiteral("weatherAssessmentApplied"), decision.weather_assessment_applied);
    result.insert(QStringLiteral("selectedCandidateId"),
                  QString::fromStdString(decision.selected_candidate_id));
    result.insert(QStringLiteral("trajectoryStatus"),
                  decision.feasibility == bluesky::planning::Feasibility::Feasible
                      ? QStringLiteral("FEASIBLE") : QStringLiteral("INFEASIBLE"));
    result.insert(QStringLiteral("weatherReady"),
                  decision.feasibility == bluesky::planning::Feasibility::Feasible
                      && decision.weather_assessment_applied);
    result.insert(QStringLiteral("weatherError"),
                  result.value(QStringLiteral("weatherReady")).toBool()
                      ? QString() : QString::fromStdString(decision.explanation));
    result.insert(QStringLiteral("weatherSnapshotId"),
                  decision.weather_snapshot_id.empty()
                      ? QString::fromStdString(snapshot.snapshot_id)
                      : QString::fromStdString(decision.weather_snapshot_id));
    result.insert(QStringLiteral("weatherSnapshotVersion"),
                  decision.weather_snapshot_version.empty()
                      ? QString::fromStdString(snapshot.snapshot_version)
                      : QString::fromStdString(decision.weather_snapshot_version));
    result.insert(QStringLiteral("totalTimeS"), decision.weather_corrected_time_s);
    result.insert(QStringLiteral("totalEnergyWh"), decision.weather_corrected_energy_wh);
    result.insert(QStringLiteral("remainingEnergyWh"), decision.weather_remaining_energy_wh);

    QVariantList rejected;
    for (const auto& item : decision.rejected_candidates) {
        QStringList violations;
        for (const auto& violation : item.violations)
            violations.push_back(QString::fromStdString(violation));
        rejected.push_back(QVariantMap{
            {QStringLiteral("candidateId"), QString::fromStdString(item.candidate_id)},
            {QStringLiteral("solverId"), QString::fromStdString(item.solver_id)},
            {QStringLiteral("violations"), violations}
        });
    }
    result.insert(QStringLiteral("rejectedCandidates"), rejected);

    // Preserve the exact typed evidence used by the orchestrator. Do not reconstruct
    // safety/NOTAM/weather attestations from serialized status strings in QML.
    const auto selectedAssessment = assessments.find(decision.selected_candidate_id);
    const auto selectedPerformance = performanceByCandidate.find(decision.selected_candidate_id);
    if (decision.feasibility == bluesky::planning::Feasibility::Feasible
        && decision.weather_assessment_applied
        && decision.selected_route.has_value()
        && decision.selected_flight_safety_validation.has_value()
        && decision.selected_notam_validation.has_value()
        && decision.selected_weather_trajectory.has_value()
        && selectedAssessment != assessments.end()
        && selectedAssessment->second.evaluated
        && selectedAssessment->second.trajectory.has_value()
        && selectedPerformance != performanceByCandidate.end()) {
        // Keep the typed decision, immutable NOTAM input, weather trajectory
        // evaluation and exact per-candidate performance input together. This
        // bundle is the hand-off to lifecycle evidence assembly; the UI map is
        // only a presentation projection and is not an evidence source.
        WeatherCandidateEvidenceBundle evidence;
        evidence.planning_decision = decision;
        evidence.notam_snapshot = notamSnapshot;
        evidence.weather_evaluation.weather_ready = selectedAssessment->second.evaluated;
        evidence.weather_evaluation.weather_error = selectedAssessment->second.error;
        evidence.weather_evaluation.trajectory = *selectedAssessment->second.trajectory;
        evidence.weather_evaluation.corrected_route = selectedAssessment->second.corrected_route;
        evidence.performance_input = selectedPerformance->second;
        evidence.vehicle_configuration_id = selectedPerformance->second.configuration_version;
        evidence.selected_candidate_id = decision.selected_candidate_id;
        evidence.selected_route = *decision.selected_route;
        evidence.flight_safety_validation = *decision.selected_flight_safety_validation;
        evidence.notam_validation = *decision.selected_notam_validation;
        evidence.weather_trajectory = *decision.selected_weather_trajectory;
        evidence.weather_snapshot_id = decision.weather_snapshot_id;
        evidence.weather_snapshot_version = decision.weather_snapshot_version;
        worker.selected_candidate_evidence = std::move(evidence);
    }
    return worker;
#endif
}

} // namespace

WeatherCalculationService::WeatherCalculationService(QObject* parent)
    : QObject(parent)
{
    connect(&m_watcher,&QFutureWatcher<WeatherCalculationWorkerResult>::finished,this,[this]() {
        const WeatherCalculationWorkerResult worker = m_watcher.result();
        m_result = worker.presentation;
        m_selectedCandidateEvidence = worker.selected_candidate_evidence;
        emit selectedCandidateEvidenceChanged();
        emit resultChanged();
        setError(m_result.value(QStringLiteral("weatherError")).toString());
        const bool ready=m_result.value(QStringLiteral("weatherReady")).toBool();
        const bool feasible=m_result.value(QStringLiteral("trajectoryStatus")).toString()
                            ==QStringLiteral("FEASIBLE");
        setState(ready ? (feasible ? QStringLiteral("READY")
                                   : QStringLiteral("ASSESSMENT_INFEASIBLE"))
                       : QStringLiteral("BLOCKED"));
        emit evaluationFinished(m_result);
    });
}

bool WeatherCalculationService::decoderAvailable() const
{
#ifdef BLUESKY_HAVE_ECCODES
    return true;
#else
    return false;
#endif
}

bool WeatherCalculationService::evaluateRoute(
    const QVariantList& forecastProducts,const QVariantMap& routeSnapshot,
    const QString& missionModelJson,const QString& missionStartUtc,
    const QVariantMap& validatedPerformanceProfile)
{
    if (m_watcher.isRunning()) {
        setError(QStringLiteral("WEATHER_ASSESSMENT_ALREADY_RUNNING"));
        return false;
    }
    if (!decoderAvailable()) {
        setError(QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
        return false;
    }
    std::vector<ForecastProduct> products;
    QString productError;
    if (!validateForecastProducts(forecastProducts,products,productError)) {
        setError(productError);
        return false;
    }
    QString vehicleId,configurationVersion,missionId,error;
    if (!validateVehicleAssignment(missionModelJson,validatedPerformanceProfile,
                                   vehicleId,configurationVersion,missionId,error)) {
        setError(error);
        return false;
    }
    Route route;
    if (!buildRoute(routeSnapshot,route,error)) {
        setError(error);
        return false;
    }
    if (route.lineage.mission_id!=missionId.toStdString()) {
        setError(QStringLiteral("MISSION_ROUTE_LINEAGE_MISMATCH"));
        return false;
    }
    TrajectoryPerformanceInput input;
    if (!buildPerformanceInput(validatedPerformanceProfile,route,missionStartUtc,input,error)) {
        setError(error);
        return false;
    }
    const QDateTime missionStart=QDateTime::fromString(missionStartUtc,Qt::ISODate).toUTC();
    if (!missionStart.isValid()) {
        setError(QStringLiteral("VALID_MISSION_START_UTC_REQUIRED"));
        return false;
    }
    const QDateTime firstValid=QDateTime::fromString(
        products.front().modelRunUtc,Qt::ISODate).toUTC().addSecs(qint64(products.front().forecastHour)*3600);
    const auto lastProduct=std::max_element(products.begin(),products.end(),
        [](const ForecastProduct& a,const ForecastProduct& b){ return a.forecastHour<b.forecastHour; });
    const QDateTime lastValid=QDateTime::fromString(
        lastProduct->modelRunUtc,Qt::ISODate).toUTC().addSecs(qint64(lastProduct->forecastHour)*3600);
    if (missionStart<firstValid || missionStart>lastValid) {
        setError(QStringLiteral("MISSION_START_OUTSIDE_DOWNLOADED_FORECAST_VALID_TIME_WINDOW"));
        return false;
    }
    setError(QString());
    setState(QStringLiteral("ASSESSING"));
    m_watcher.setFuture(QtConcurrent::run(
        [products,route,input]() {
            WeatherCalculationWorkerResult worker;
            worker.presentation = evaluateOnWorker(products,route,input);
            return worker;
        }));
    return true;
}



bool WeatherCalculationService::evaluateCandidates(
    const QVariantList& forecastProducts,
    const QVariantList& candidateSnapshots,
    const QString& missionModelJson,
    const QString& missionStartUtc,
    const QVariantMap& validatedPerformanceProfile,
    const QStringList& objectivePriorities,
    const QVariantMap& normalizedNotamSnapshot)
{
    if (m_watcher.isRunning()) {
        setError(QStringLiteral("WEATHER_ASSESSMENT_ALREADY_RUNNING"));
        return false;
    }

    bluesky::planning::NotamSnapshot notamSnapshot;
    double notamSafetyBufferM = 0.0;
    QString notamSnapshotError;
    if (!parseNormalizedNotamSnapshot(normalizedNotamSnapshot, notamSnapshot,
                                      notamSafetyBufferM, notamSnapshotError)) {
        setError(notamSnapshotError);
        return false;
    }
    if (!decoderAvailable()) {
        setError(QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
        return false;
    }
    if (candidateSnapshots.isEmpty()) {
        setError(QStringLiteral("WEATHER_CANDIDATE_SET_REQUIRED"));
        return false;
    }
    if (candidateSnapshots.size() > 64) {
        setError(QStringLiteral("WEATHER_CANDIDATE_SET_EXCEEDS_64_ROUTES"));
        return false;
    }
    if (objectivePriorities.isEmpty()) {
        setError(QStringLiteral("EXPLICIT_ROUTE_OBJECTIVE_PRIORITIES_REQUIRED"));
        return false;
    }

    const QStringList weatherRankablePriorities{
        QStringLiteral("completion_time"), QStringLiteral("time"), QStringLiteral("fast"),
        QStringLiteral("energy"), QStringLiteral("endurance"), QStringLiteral("reserve")
    };
    for (const QString& priority : objectivePriorities) {
        if (!weatherRankablePriorities.contains(priority.trimmed())) {
            setError(QStringLiteral("WEATHER_RERANKING_REQUIRES_TIME_ENERGY_OR_RESERVE_OBJECTIVE"));
            return false;
        }
    }

    std::vector<ForecastProduct> products;
    QString error;
    if (!validateForecastProducts(forecastProducts, products, error)) {
        setError(error);
        return false;
    }

    QString vehicleId, configurationVersion, missionId;
    if (!validateVehicleAssignment(missionModelJson, validatedPerformanceProfile,
                                   vehicleId, configurationVersion, missionId, error)) {
        setError(error);
        return false;
    }
    bluesky::planning::FlightSafetyEnvelope flightSafetyEnvelope;
    if (!buildFlightSafetyEnvelope(validatedPerformanceProfile, flightSafetyEnvelope, error)) {
        setError(error);
        return false;
    }

    if (!missionGeometryWithinNotamQueryCoverage(missionModelJson, notamSnapshot, notamSafetyBufferM)) {
        setError(QStringLiteral("NOTAM_SNAPSHOT_DOES_NOT_COVER_MISSION_GEOMETRY_WITH_BUFFER"));
        return false;
    }

    const QDateTime missionStart = QDateTime::fromString(missionStartUtc, Qt::ISODate).toUTC();
    if (!missionStart.isValid()) {
        setError(QStringLiteral("VALID_MISSION_START_UTC_REQUIRED"));
        return false;
    }
    const QDateTime firstValid = QDateTime::fromString(
        products.front().modelRunUtc, Qt::ISODate).toUTC()
        .addSecs(qint64(products.front().forecastHour) * 3600);
    const auto lastProduct = std::max_element(products.begin(), products.end(),
        [](const ForecastProduct& a, const ForecastProduct& b) {
            return a.forecastHour < b.forecastHour;
        });
    const QDateTime lastValid = QDateTime::fromString(
        lastProduct->modelRunUtc, Qt::ISODate).toUTC()
        .addSecs(qint64(lastProduct->forecastHour) * 3600);
    if (missionStart < firstValid || missionStart > lastValid) {
        setError(QStringLiteral("MISSION_START_OUTSIDE_DOWNLOADED_FORECAST_VALID_TIME_WINDOW"));
        return false;
    }

    std::vector<bluesky::planning::CandidateSolution> candidates;
    candidates.reserve(static_cast<std::size_t>(candidateSnapshots.size()));
    std::map<std::string, TrajectoryPerformanceInput> performanceByCandidate;
    std::set<std::string> candidateIds;
    QString missionVersion;
    QString calculationInputVersion;
    QString commonSafetyEnvelopeId;
    for (const QVariant& value : candidateSnapshots) {
        const QVariantMap snapshot = value.toMap();
        Route route;
        if (!buildRoute(snapshot, route, error)) {
            setError(error);
            return false;
        }
        if (!route.constraints.avoid_restricted_areas) {
            setError(QStringLiteral("NOTAM_AVOIDANCE_REQUIRED_FOR_WEATHER_RERANKING"));
            return false;
        }
        if (route.environment.airspace_snapshot_id != notamSnapshot.snapshot_id) {
            setError(QStringLiteral("ROUTE_NOTAM_SNAPSHOT_IDENTITY_MISMATCH"));
            return false;
        }
        if (!routeWithinNotamQueryCoverage(route, notamSnapshot, notamSafetyBufferM)) {
            setError(QStringLiteral("NOTAM_SNAPSHOT_DOES_NOT_COVER_ROUTE_WITH_BUFFER"));
            return false;
        }
        if (route.lineage.mission_id != missionId.toStdString()) {
            setError(QStringLiteral("MISSION_ROUTE_LINEAGE_MISMATCH"));
            return false;
        }
        const QString currentMissionVersion =
            QString::fromStdString(route.lineage.mission_version);
        const QString currentInputVersion =
            QString::fromStdString(route.lineage.calculation_input_version);
        if (missionVersion.isEmpty()) {
            missionVersion = currentMissionVersion;
            calculationInputVersion = currentInputVersion;
        } else if (missionVersion != currentMissionVersion
                   || calculationInputVersion != currentInputVersion) {
            setError(QStringLiteral("WEATHER_CANDIDATES_MUST_SHARE_MISSION_AND_CALCULATION_INPUT_VERSION"));
            return false;
        }

        // Legacy ALLOWED flags are not safety evidence. Retain only identity cross-checks;
        // the shared typed validator below is the authoritative admission gate.
        if (snapshot.value(QStringLiteral("flightSafetyVehicleId")).toString().trimmed()
            != vehicleId) {
            setError(QStringLiteral("CANDIDATE_FLIGHT_SAFETY_VEHICLE_MISMATCH"));
            return false;
        }
        const QString currentSafetyEnvelopeId =
            snapshot.value(QStringLiteral("flightSafetyEnvelopeId")).toString().trimmed();
        if (currentSafetyEnvelopeId != validatedPerformanceProfile.value(
                QStringLiteral("flightSafetyEnvelopeId")).toString().trimmed()) {
            setError(QStringLiteral("CANDIDATE_FLIGHT_SAFETY_ENVELOPE_MISMATCH"));
            return false;
        }
        if (commonSafetyEnvelopeId.isEmpty())
            commonSafetyEnvelopeId = currentSafetyEnvelopeId;
        else if (commonSafetyEnvelopeId != currentSafetyEnvelopeId) {
            setError(QStringLiteral("WEATHER_CANDIDATES_USE_DIFFERENT_FLIGHT_SAFETY_ENVELOPES"));
            return false;
        }

        QString candidateId = snapshot.value(QStringLiteral("candidateId")).toString().trimmed();
        if (candidateId.isEmpty())
            candidateId = QString::fromStdString(route.lineage.route_id);
        if (!candidateIds.insert(candidateId.toStdString()).second) {
            setError(QStringLiteral("DUPLICATE_WEATHER_CANDIDATE_ID"));
            return false;
        }

        // Bind the performance input to this exact canonical route. Candidate
        // speed profiles are route-specific and must never be shared across the set.
        TrajectoryPerformanceInput candidatePerformance;
        if (!buildPerformanceInput(validatedPerformanceProfile, route, missionStartUtc,
                                   candidatePerformance, error)) {
            setError(error);
            return false;
        }
        performanceByCandidate.emplace(candidateId.toStdString(),
                                        std::move(candidatePerformance));

        double nominalTime = 0.0;
        double nominalEnergy = 0.0;
        double nominalReserve = 0.0;
        double objectiveScore = 0.0;
        if (!readFinite(snapshot, QStringLiteral("estimatedTimeS"), nominalTime)
            || nominalTime <= 0.0
            || !readFinite(snapshot, QStringLiteral("estimatedEnergyWh"), nominalEnergy)
            || nominalEnergy < 0.0
            || !readFinite(snapshot, QStringLiteral("estimatedReserveWh"), nominalReserve)
            || nominalReserve < 0.0
            || !readFinite(snapshot, QStringLiteral("objectiveScore"), objectiveScore)) {
            setError(QStringLiteral("CANDIDATE_NOMINAL_TIME_ENERGY_RESERVE_AND_OBJECTIVE_REQUIRED"));
            return false;
        }

        bluesky::planning::CandidateSolution candidate;
        candidate.candidate_id = candidateId.toStdString();
        candidate.solver_id = "hmi-weather-candidate-set";
        candidate.solver_version = "1";
        candidate.estimated_time_s = nominalTime;
        candidate.estimated_energy_wh = nominalEnergy;
        candidate.estimated_reserve_wh = nominalReserve;
        candidate.objective_score = objectiveScore;
        candidate.feasibility = bluesky::planning::Feasibility::Feasible;
        for (const auto& waypoint : route.waypoints)
            candidate.route_elements.push_back(waypoint.waypoint_id);
        candidate.trajectory_route = std::move(route);
        bluesky::planning::attachFlightSafetyDynamics(candidate, *candidate.trajectory_route);
        candidates.push_back(std::move(candidate));
    }

    bluesky::planning::MissionProblem problem;
    problem.mission_id = missionId.toStdString();
    problem.mission_version = missionVersion.toStdString();
    problem.problem_class = "point_to_point";
    problem.vehicle_ids = {vehicleId.toStdString()};
    problem.environment_version = calculationInputVersion.toStdString();
    problem.requires_flight_safety_validation = true;
    problem.flight_safety_vehicle_id = vehicleId.toStdString();
    problem.flight_safety_envelope = flightSafetyEnvelope;
    problem.notam_snapshot = std::make_shared<const bluesky::planning::NotamSnapshot>(notamSnapshot);
    problem.requires_notam_validation = true;
    for (const auto& priority : objectivePriorities)
        problem.objective_priorities.push_back(priority.trimmed().toStdString());

    setError(QString());
    setState(QStringLiteral("ASSESSING_CANDIDATE_SET"));
    m_watcher.setFuture(QtConcurrent::run(
        [products, candidates, problem, performanceByCandidate, notamSnapshot]() {
            return evaluateCandidateSetOnWorker(
                products, candidates, problem, performanceByCandidate, notamSnapshot);
        }));
    return true;
}

void WeatherCalculationService::setState(const QString& state)
{
    if (m_state==state) return;
    m_state=state;
    emit stateChanged();
}

void WeatherCalculationService::setError(const QString& error)
{
    if (m_lastError!=error) {
        m_lastError=error;
        emit lastErrorChanged();
    }
    if (!error.isEmpty())
        setState(QStringLiteral("BLOCKED"));
}
