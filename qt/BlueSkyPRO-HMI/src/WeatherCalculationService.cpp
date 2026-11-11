#include "WeatherCalculationService.h"

#include "04_SOFTWARE/PLANNING/model/route_model.hpp"
#include "04_SOFTWARE/PLANNING/wind_performance_trajectory.hpp"
#include "04_SOFTWARE/WEATHER/grib2_weather_pipeline.hpp"
#include "04_SOFTWARE/WEATHER/weather_candidate_assessor.hpp"
#include "04_SOFTWARE/PLANNING/orchestrator/algorithm_orchestrator.hpp"

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
    route.trajectory_geometry_complete=true;
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

bool buildPerformanceInput(const QVariantMap& profile,const QString& missionStartUtc,
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

QVariantMap evaluateCandidateSetOnWorker(
    const std::vector<ForecastProduct>& products,
    const std::vector<bluesky::planning::CandidateSolution>& candidates,
    const bluesky::planning::MissionProblem& problem,
    const TrajectoryPerformanceInput& input)
{
    QVariantMap result;
#ifndef BLUESKY_HAVE_ECCODES
    Q_UNUSED(products);
    Q_UNUSED(candidates);
    Q_UNUSED(problem);
    Q_UNUSED(input);
    result.insert(QStringLiteral("weatherReady"), false);
    result.insert(QStringLiteral("decodeSucceeded"), false);
    result.insert(QStringLiteral("weatherError"), QStringLiteral("GRIB2_DECODER_NOT_AVAILABLE"));
    result.insert(QStringLiteral("flightReleaseEligible"), false);
    return result;
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
            return result;
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
                return result;
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
            return result;
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
    result.insert(QStringLiteral("assessmentScope"), QStringLiteral("WEATHER_RERANKING_ONLY"));
    if (!snapshot.decode_succeeded || snapshot.fields.empty()) {
        result.insert(QStringLiteral("weatherReady"), false);
        result.insert(QStringLiteral("weatherError"),
                      QString::fromStdString(snapshot.error.empty()
                          ? std::string("DECODED_WIND_SNAPSHOT_EMPTY") : snapshot.error));
        return result;
    }

    auto baseAssessor = bluesky::weather::makeCandidateWeatherAssessor(
        snapshot.fields, input, snapshot.snapshot_id, snapshot.snapshot_version,
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
                  QString::fromStdString(decision.weather_snapshot_id));
    result.insert(QStringLiteral("weatherSnapshotVersion"),
                  QString::fromStdString(decision.weather_snapshot_version));
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
    return result;
#endif
}

} // namespace

WeatherCalculationService::WeatherCalculationService(QObject* parent)
    : QObject(parent)
{
    connect(&m_watcher,&QFutureWatcher<QVariantMap>::finished,this,[this]() {
        m_result=m_watcher.result();
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
    if (!buildPerformanceInput(validatedPerformanceProfile,missionStartUtc,input,error)) {
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
            return evaluateOnWorker(products,route,input);
        }));
    return true;
}



bool WeatherCalculationService::evaluateCandidates(
    const QVariantList& forecastProducts,
    const QVariantList& candidateSnapshots,
    const QString& missionModelJson,
    const QString& missionStartUtc,
    const QVariantMap& validatedPerformanceProfile,
    const QStringList& objectivePriorities)
{
    if (m_watcher.isRunning()) {
        setError(QStringLiteral("WEATHER_ASSESSMENT_ALREADY_RUNNING"));
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
    if (objectivePriorities.isEmpty()) {
        setError(QStringLiteral("EXPLICIT_ROUTE_OBJECTIVE_PRIORITIES_REQUIRED"));
        return false;
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

    TrajectoryPerformanceInput input;
    if (!buildPerformanceInput(validatedPerformanceProfile, missionStartUtc, input, error)) {
        setError(error);
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
    std::set<std::string> candidateIds;
    QString missionVersion;
    QString calculationInputVersion;
    for (const QVariant& value : candidateSnapshots) {
        const QVariantMap snapshot = value.toMap();
        Route route;
        if (!buildRoute(snapshot, route, error)) {
            setError(error);
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

        // Weather ranking must never bypass the already completed dynamic flight-safety gate.
        if (snapshot.value(QStringLiteral("flightSafetyValidationState")).toString()
                != QStringLiteral("ALLOWED")
            || snapshot.value(QStringLiteral("flightSafetyEnvelopeId")).toString().trimmed().isEmpty()) {
            setError(QStringLiteral("CANDIDATE_REQUIRES_MATCHED_FLIGHT_SAFETY_VALIDATION"));
            return false;
        }

        QString candidateId = snapshot.value(QStringLiteral("candidateId")).toString().trimmed();
        if (candidateId.isEmpty())
            candidateId = QString::fromStdString(route.lineage.route_id);
        if (!candidateIds.insert(candidateId.toStdString()).second) {
            setError(QStringLiteral("DUPLICATE_WEATHER_CANDIDATE_ID"));
            return false;
        }

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
        candidates.push_back(std::move(candidate));
    }

    bluesky::planning::MissionProblem problem;
    problem.mission_id = missionId.toStdString();
    problem.mission_version = missionVersion.toStdString();
    problem.problem_class = "point_to_point";
    problem.vehicle_ids = {vehicleId.toStdString()};
    problem.environment_version = calculationInputVersion.toStdString();
    for (const auto& priority : objectivePriorities)
        problem.objective_priorities.push_back(priority.trimmed().toStdString());

    setError(QString());
    setState(QStringLiteral("ASSESSING_CANDIDATE_SET"));
    m_watcher.setFuture(QtConcurrent::run(
        [products, candidates, problem, input]() {
            return evaluateCandidateSetOnWorker(products, candidates, problem, input);
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
