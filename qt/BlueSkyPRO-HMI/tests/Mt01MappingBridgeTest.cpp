#include "Mt01MappingBridge.h"
#include "NotamGeometryEngine.h"
#include "mission_lifecycle.hpp"

#include <QCoreApplication>
#include <cstdio>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cassert>

static QString validMissionJson()
{
    QJsonArray coordinates{
        QJsonArray{0.0, 0.0}, QJsonArray{0.002, 0.0},
        QJsonArray{0.002, 0.002}, QJsonArray{0.0, 0.002}
    };
    QJsonObject aoi{
        {"geometryId", "AOI-01"},
        {"geometryType", "POLYGON_AOI"},
        {"coordinateReferenceSystem", "EPSG:4326"},
        {"templateIds", QJsonArray{"MT-01"}},
        {"coordinates", coordinates},
        {"version", 1}
    };
    QJsonObject mission{
        {"missionId", "MT01-MAVIC3E-HMI-TEST"},
        {"missionVersion", "1"},
        {"missionMode", "AUTO"},
        {"selectedTemplateIds", QJsonArray{"MT-01", "MT-02"}},
        {"task", QJsonObject{{"description", "Photogrammetry test"}}},
        {"intent", QJsonObject{{"objective", "Orthomosaic"}, {"taskType", "MAPPING"}}},
        {"vehicleAssignments", QJsonArray{QJsonObject{
            {"vehicleId", "DJI-MAVIC-3E"},
            {"configurationId", "MAVIC-3E-STANDARD"},
            {"configurationVersion", "MAVIC-3E-STANDARD-REFERENCE-001"},
            {"configurationType", "STANDARD"},
            {"validationState", "APPROVED"},
            {"approvalState", "APPROVED"}
        }}},
        {"equipmentAssignments", QJsonArray{QJsonObject{
            {"vehicleId", "DJI-MAVIC-3E"},
            {"equipmentId", "DJI-MAVIC-3E-WIDE"},
            {"equipmentProfileId", "DJI-MAVIC-3E-WIDE-PROFILE"},
            {"equipmentClass", "CAMERA"},
            {"configurationVersion", "MAVIC-3E-WIDE-REFERENCE-001"},
            {"compatibilityState", "COMPATIBLE"},
            {"verificationState", "VERIFIED"},
            {"calibrationState", "NOT_VERIFIED"}
        }}},
        {"geometryObjects", QJsonArray{aoi}},
        {"operations", QJsonArray{QJsonObject{
            {"operationId", "OP-MT01-AOI-01"},
            {"templateId", "MT-01"},
            {"operationType", "MAPPING_SWATH"},
            {"geometryId", "AOI-01"},
            {"equipmentIds", QJsonArray{"DJI-MAVIC-3E-WIDE"}},
            {"executionEstimate", QJsonObject{{"state", "UNRESOLVED"}}}
        }}}
    };
    return QString::fromUtf8(QJsonDocument(mission).toJson(QJsonDocument::Compact));
}

static QVariantMap validParameters()
{
    return {
        {"focalLengthMm", 12.29},
        {"pixelPitchUm", 3.3},
        {"imageWidthPx", 5280},
        {"imageHeightPx", 3956},
        {"targetGsdCmPx", 3.0},
        {"forwardOverlapPercent", 80.0},
        {"sideOverlapPercent", 70.0},
        {"minimumAltitudeAglM", 50.0},
        {"maximumAltitudeAglM", 150.0},
        {"cruiseSpeedMps", 10.0},
        {"lineAngleDeg", 0.0},
        {"cameraConfigurationId", "DJI-MAVIC-3E-WIDE"},
        {"calibrationSourceReference", "DJI Mavic 3 Enterprise manufacturer specifications"},
        {"intrinsicCalibrationVerified", false},
        {"notamSnapshotId", "FAA-NMS-TEST-SNAPSHOT"},
        {"notamSourceId", "FAA_NMS_TEST"},
        {"notamSourceVersion", "FAA-NMS-TEST-SNAPSHOT"},
        {"notamSnapshotEpochS", 1800000000LL},
        {"notamQueryLatitude", 0.001},
        {"notamQueryLongitude", 0.001},
        {"notamQueryRadiusM", 500.0},
        {"plannedMissionStartUtc", "2027-01-15T09:00:00Z"},
        {"plannedMissionEndUtc", "2027-01-15T10:00:00Z"},
        {"notamSafetyBufferM", 25.0},
        {"notamRecords", QVariantList{}}
    };
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Mt01MappingBridge bridge;

    const QVariantMap normalizedEmptySnapshot = bridge.normalizeNotamSnapshot(validParameters());
    assert(normalizedEmptySnapshot.value("valid").toBool());
    assert(normalizedEmptySnapshot.value("snapshotId").toString() == "FAA-NMS-TEST-SNAPSHOT");
    assert(normalizedEmptySnapshot.value("zones").toList().isEmpty());
    assert(normalizedEmptySnapshot.value("coverageRadiusM").toDouble() == 500.0);

    const QVariantMap result = bridge.calculate(validMissionJson(), validParameters());
    assert(result.value("status").toString() == "COVERAGE_CANDIDATE_AVAILABLE");
    assert(result.value("coverageCandidateAvailable").toBool());
    assert(!result.value("flightReleaseEligible").toBool());
    assert(!result.value("taskExecutionEstimatesReady").toBool());
    assert(!result.value("taskExecutionEstimatesAppliedToTrajectory").toBool());

    const QVariantMap lifecycleIdentity = result.value("lifecycleIdentity").toMap();
    bluesky::planning::MissionLifecycleRecord mt01LifecycleRecord;
    mt01LifecycleRecord.mission_id = lifecycleIdentity.value("missionId").toString().toStdString();
    mt01LifecycleRecord.mission_version = lifecycleIdentity.value("missionVersion").toString().toStdString();
    for (const QVariant &templateId : lifecycleIdentity.value("templateIds").toList())
        mt01LifecycleRecord.mission_template_ids.push_back(templateId.toString().toStdString());
    assert(mt01LifecycleRecord.mission_id == "MT01-MAVIC3E-HMI-TEST");
    assert(mt01LifecycleRecord.mission_version == "1");
    assert(mt01LifecycleRecord.mission_template_ids.size() == 2);
    assert(std::find(mt01LifecycleRecord.mission_template_ids.begin(),
                     mt01LifecycleRecord.mission_template_ids.end(), "MT-01")
           != mt01LifecycleRecord.mission_template_ids.end());
    bluesky::planning::MissionLifecycleEvidence mt01LifecycleEvidence;
    mt01LifecycleEvidence.mt01_coverage_candidate_available =
        result.value("coverageCandidateAvailable").toBool();
    mt01LifecycleEvidence.mt01_flight_release_eligible =
        result.value("flightReleaseEligible").toBool();
    mt01LifecycleEvidence.mt01_task_execution_estimates_ready =
        result.value("taskExecutionEstimatesReady").toBool();
    mt01LifecycleEvidence.mt01_task_execution_estimates_applied_to_trajectory =
        result.value("taskExecutionEstimatesAppliedToTrajectory").toBool();
    mt01LifecycleEvidence.mt01_camera_intrinsic_calibration_verified =
        result.value("cameraCalibrationVerified").toBool();
    const auto mt01ReadinessBlockers =
        bluesky::planning::MissionLifecycleController::mt01ReadinessBlockers(
            mt01LifecycleRecord, mt01LifecycleEvidence);
    assert(mt01ReadinessBlockers.size() == 4);
    assert(std::find(mt01ReadinessBlockers.begin(), mt01ReadinessBlockers.end(),
                     "MT01_FLIGHT_RELEASE_NOT_ELIGIBLE") != mt01ReadinessBlockers.end());
    assert(std::find(mt01ReadinessBlockers.begin(), mt01ReadinessBlockers.end(),
                     "MT01_TASK_EXECUTION_ESTIMATES_NOT_READY") != mt01ReadinessBlockers.end());
    assert(std::find(mt01ReadinessBlockers.begin(), mt01ReadinessBlockers.end(),
                     "MT01_TASK_EXECUTION_ESTIMATES_NOT_APPLIED") != mt01ReadinessBlockers.end());
    assert(std::find(mt01ReadinessBlockers.begin(), mt01ReadinessBlockers.end(),
                     "MT01_CAMERA_INTRINSIC_CALIBRATION_NOT_VERIFIED") != mt01ReadinessBlockers.end());

    assert(result.value("findings").toStringList().join(";")
               .contains("MT01_OPERATION_EXECUTION_ESTIMATE_UNRESOLVED"));

    QJsonObject validatedEstimateMission = QJsonDocument::fromJson(
        validMissionJson().toUtf8()).object();
    QJsonArray validatedOperations = validatedEstimateMission.value("operations").toArray();
    QJsonObject validatedOperation = validatedOperations.first().toObject();
    validatedOperation.insert("executionEstimate", QJsonObject{
        {"state", "VALIDATED"},
        {"durationS", 120.0},
        {"energyWh", 8.5},
        {"sourceType", "OPERATOR_APPROVED"},
        {"sourceId", "MT01-ESTIMATE-001"},
        {"sourceVersion", "1"},
        {"methodVersion", "1"},
        {"vehicleId", "DJI-MAVIC-3E"},
        {"vehicleConfigurationVersion", "MAVIC-3E-STANDARD-REFERENCE-001"}
    });
    validatedOperations.replace(0, validatedOperation);
    validatedEstimateMission.insert("operations", validatedOperations);
    const QVariantMap validatedEstimateResult = bridge.calculate(
        QString::fromUtf8(QJsonDocument(validatedEstimateMission).toJson(QJsonDocument::Compact)),
        validParameters());
    assert(validatedEstimateResult.value("coverageCandidateAvailable").toBool());
    assert(validatedEstimateResult.value("taskExecutionEstimatesReady").toBool());
    assert(!validatedEstimateResult.value("taskExecutionEstimatesAppliedToTrajectory").toBool());
    assert(validatedEstimateResult.value("findings").toStringList().join(";")
               .contains("MT01_OPERATION_EXECUTION_ESTIMATE_NOT_APPLIED_TO_TRAJECTORY"));

    auto validatedEstimateLifecycleEvidence = mt01LifecycleEvidence;
    validatedEstimateLifecycleEvidence.mt01_task_execution_estimates_ready =
        validatedEstimateResult.value("taskExecutionEstimatesReady").toBool();
    validatedEstimateLifecycleEvidence.mt01_task_execution_estimates_applied_to_trajectory =
        validatedEstimateResult.value("taskExecutionEstimatesAppliedToTrajectory").toBool();
    const auto validatedEstimateBlockers =
        bluesky::planning::MissionLifecycleController::mt01ReadinessBlockers(
            mt01LifecycleRecord, validatedEstimateLifecycleEvidence);
    assert(std::find(validatedEstimateBlockers.begin(), validatedEstimateBlockers.end(),
                     "MT01_TASK_EXECUTION_ESTIMATES_NOT_READY") == validatedEstimateBlockers.end());
    assert(std::find(validatedEstimateBlockers.begin(), validatedEstimateBlockers.end(),
                     "MT01_TASK_EXECUTION_ESTIMATES_NOT_APPLIED") != validatedEstimateBlockers.end());
    assert(result.value("coverageValid").toBool());
    assert(result.value("waypoints").toList().size() >= 2);
    assert(!result.value("capturePoints").toList().isEmpty());
    const QVariantMap metrics = result.value("metrics").toMap();
    assert(metrics.value("coverageLineCount").toULongLong() > 0);
    assert(metrics.value("calculatedAltitudeAglM").toDouble() > 100.0);

    assert(result.value("notamSnapshotId").toString() == "FAA-NMS-TEST-SNAPSHOT");
    assert(result.value("metrics").toMap().value("notamApplicableComponentCount").toULongLong() == 0);

    QVariantMap activeNotamParameters = validParameters();
    QVariantMap activeNotamSource{
        {"id", "N-HMI-001"},
        {"effectiveStart", "2027-01-15T08:00:00Z"},
        {"effectiveEnd", "2027-01-15T11:00:00Z"},
        {"lowerLimitRaw", "GND"},
        {"upperLimitRaw", "150M AGL"},
        {"originalText", "E) AIRSPACE CLSD WI AREA: 0008N00008E-0008N00012E-0012N00012E-0012N00008E-0008N00008E\nF)GND\nG)150M AGL"},
        {"geometryJson", QStringLiteral(R"({"type":"Polygon","coordinates":[[[0.0008,0.0008],[0.0012,0.0008],[0.0012,0.0012],[0.0008,0.0012],[0.0008,0.0008]]]})")}
    };
    const QVariantMap activeNotam = NotamGeometryEngine::normalize(activeNotamSource);
    assert(activeNotam.value("notamInterpretationVersion").toInt() == 1);
    assert(activeNotam.value("restrictionClass").toString() == "PROHIBITED");
    assert(activeNotam.value("geometryType").toString() == "Polygon");
    assert(activeNotam.value("geometryValidated").toBool());
    assert(activeNotam.value("geometryProvenance").toString() == "SOURCE_GEOJSON");
    assert(activeNotam.value("interpretationState").toString() == "CONFIRMED");
    assert(activeNotam.value("planningComponents").toList().size() == 1);
    activeNotamParameters.insert("notamRecords", QVariantList{activeNotam});
    const QVariantMap normalizedActiveSnapshot =
        bridge.normalizeNotamSnapshot(activeNotamParameters);
    assert(normalizedActiveSnapshot.value("valid").toBool());
    const QVariantList normalizedZones = normalizedActiveSnapshot.value("zones").toList();
    assert(normalizedZones.size() == 1);
    assert(normalizedZones.first().toMap().value("componentId").toString()
           == "N-HMI-001-COMP-1");
    const QVariantMap notamResult = bridge.calculate(validMissionJson(), activeNotamParameters);
    assert(notamResult.value("status").toString() == "COVERAGE_CANDIDATE_AVAILABLE");
    assert(notamResult.value("metrics").toMap().value("notamApplicableComponentCount").toULongLong() == 1);
    assert(notamResult.value("notamApplicableIds").toStringList().contains("N-HMI-001"));
    assert(notamResult.value("notamComponentIds").toStringList().contains("N-HMI-001-COMP-1"));

    // The map retains the complete source polygon including its hole;
    // planning must block until the planner supports hole-aware topology.
    QVariantMap holeParameters = activeNotamParameters;
    QVariantMap holeSource = activeNotamSource;
    holeSource.insert("geometryJson", QStringLiteral(
        R"({"type":"Polygon","coordinates":[[[0.0008,0.0008],[0.0012,0.0008],[0.0012,0.0012],[0.0008,0.0012],[0.0008,0.0008]],[[0.0009,0.0009],[0.0011,0.0009],[0.0011,0.0011],[0.0009,0.0011],[0.0009,0.0009]]]})"));
    const QVariantMap holeNotam = NotamGeometryEngine::normalize(holeSource);
    const QJsonObject holeMapGeometry = QJsonDocument::fromJson(
        holeNotam.value("geometryJson").toString().toUtf8()).object();
    assert(holeMapGeometry.value("coordinates").toArray().size() == 2);
    assert(holeNotam.value("planningComponents").toList().first().toMap()
               .value("interiorRingCount").toInt() == 1);
    holeParameters.insert("notamRecords", QVariantList{holeNotam});
    const QVariantMap holeResult = bridge.calculate(validMissionJson(), holeParameters);
    assert(holeResult.value("status").toString() == "NEEDS_INPUT");
    assert(holeResult.value("findings").toStringList().join(";")
               .contains("NOTAM_INTERIOR_RINGS_UNSUPPORTED_FOR_PLANNING"));

    QVariantMap conditionalSource = activeNotamSource;
    conditionalSource.insert("originalText",
        "E) RESTRICTED AREA WI AREA\nF)GND\nG)150M AGL");
    const QVariantMap conditionalNotam =
        NotamGeometryEngine::normalize(conditionalSource);
    assert(conditionalNotam.value("restrictionClass").toString() == "CONDITIONAL");
    QVariantMap conditionalParameters = activeNotamParameters;
    conditionalParameters.insert("notamRecords", QVariantList{conditionalNotam});
    const QVariantMap conditionalResult =
        bridge.calculate(validMissionJson(), conditionalParameters);
    assert(conditionalResult.value("status").toString() == "NEEDS_INPUT");
    assert(conditionalResult.value("findings").toStringList().join(";")
               .contains("NOTAM_RESTRICTION_SEMANTICS_UNRESOLVED"));

    QVariantMap unsupportedDatumParameters = activeNotamParameters;
    QVariantMap unsupportedDatumSource = activeNotamSource;
    unsupportedDatumSource.insert("lowerLimitRaw", "50M AMSL");
    unsupportedDatumSource.insert("upperLimitRaw", "150M AMSL");
    unsupportedDatumSource.insert("originalText",
        "E) AIRSPACE CLSD WI AREA\nF)50M AMSL\nG)150M AMSL");
    const QVariantMap unsupportedDatum =
        NotamGeometryEngine::normalize(unsupportedDatumSource);
    unsupportedDatumParameters.insert("notamRecords", QVariantList{unsupportedDatum});
    const QVariantMap unsupportedDatumResult =
        bridge.calculate(validMissionJson(), unsupportedDatumParameters);
    assert(unsupportedDatumResult.value("status").toString() == "NEEDS_INPUT");
    assert(unsupportedDatumResult.value("findings").toStringList().join(";")
               .contains("NOTAM_ALTITUDE_DATUM_UNSUPPORTED"));

    QVariantMap incompleteVerticalParameters = activeNotamParameters;
    QVariantMap incompleteVerticalSource = activeNotamSource;
    incompleteVerticalSource.insert("upperLimitRaw", QString());
    incompleteVerticalSource.insert("originalText", "E) AIRSPACE CLSD WI AREA\nF)GND");
    const QVariantMap incompleteVertical =
        NotamGeometryEngine::normalize(incompleteVerticalSource);
    incompleteVerticalParameters.insert("notamRecords", QVariantList{incompleteVertical});
    const QVariantMap incompleteVerticalResult =
        bridge.calculate(validMissionJson(), incompleteVerticalParameters);
    assert(incompleteVerticalResult.value("status").toString() == "NEEDS_INPUT");
    assert(incompleteVerticalResult.value("findings").toStringList().join(";")
               .contains("NOTAM_RESTRICTION_GEOMETRY_OR_VERTICAL_BOUNDS_UNVALIDATED"));

    QVariantMap outsideCoverageParameters = validParameters();
    outsideCoverageParameters.insert("notamQueryRadiusM", 50.0);
    const QVariantMap outsideCoverageResult =
        bridge.calculate(validMissionJson(), outsideCoverageParameters);
    assert(outsideCoverageResult.value("status").toString() == "NEEDS_INPUT");
    assert(outsideCoverageResult.value("findings").toStringList().join(";")
               .contains("NOTAM_SNAPSHOT_DOES_NOT_COVER_AOI_WITH_BUFFER"));

    QVariantMap missingCoverageMetadata = validParameters();
    missingCoverageMetadata.remove("notamQueryRadiusM");
    const QVariantMap missingCoverageResult =
        bridge.calculate(validMissionJson(), missingCoverageMetadata);
    assert(missingCoverageResult.value("status").toString() == "NEEDS_INPUT");
    assert(missingCoverageResult.value("findings").toStringList().contains(
        "NOTAM_SNAPSHOT_COVERAGE_METADATA_REQUIRED"));

    QVariantMap missingNotamSnapshot = validParameters();
    missingNotamSnapshot.remove("notamSnapshotId");
    const QVariantMap missingSnapshotResult =
        bridge.calculate(validMissionJson(), missingNotamSnapshot);
    const QVariantMap missingNormalizedSnapshot =
        bridge.normalizeNotamSnapshot(missingNotamSnapshot);
    assert(!missingNormalizedSnapshot.value("valid").toBool());
    assert(missingNormalizedSnapshot.value("errors").toStringList().contains(
        "NOTAM_SNAPSHOT_IDENTITY_AND_RECORDS_REQUIRED"));
    assert(missingSnapshotResult.value("status").toString() == "NEEDS_INPUT");
    assert(missingSnapshotResult.value("findings").toStringList().contains(
        "NOTAM_SNAPSHOT_IDENTITY_AND_RECORDS_REQUIRED"));

    QJsonObject missingOperationsFieldMission =
        QJsonDocument::fromJson(validMissionJson().toUtf8()).object();
    missingOperationsFieldMission.remove("operations");
    const QVariantMap missingOperationsField = bridge.calculate(
        QString::fromUtf8(QJsonDocument(missingOperationsFieldMission).toJson(QJsonDocument::Compact)),
        validParameters());
    assert(missingOperationsField.value("status").toString() == "NEEDS_INPUT");
    assert(missingOperationsField.value("findings").toStringList().contains(
        "MISSION_OPERATIONS_INVALID"));

    QJsonObject emptyOperationsMission =
        QJsonDocument::fromJson(validMissionJson().toUtf8()).object();
    emptyOperationsMission.insert("operations", QJsonArray{});
    const QVariantMap missingOperation = bridge.calculate(
        QString::fromUtf8(QJsonDocument(emptyOperationsMission).toJson(QJsonDocument::Compact)),
        validParameters());
    assert(missingOperation.value("status").toString() == "NEEDS_INPUT");
    assert(missingOperation.value("findings").toStringList().contains(
        "MT01_MAPPING_SWATH_OPERATION_REQUIRED"));

    QJsonObject validatedOperationMission =
        QJsonDocument::fromJson(validMissionJson().toUtf8()).object();
    QJsonObject validatedOperation = validatedOperationMission.value("operations").toArray().first().toObject();
    validatedOperation.insert("executionEstimate", QJsonObject{
        {"state", "VALIDATED"},
        {"durationS", 600.0},
        {"energyWh", 24.0},
        {"sourceType", "EQUIPMENT_MODEL"},
        {"sourceId", "CAMERA-PERFORMANCE-01"},
        {"sourceVersion", "2"},
        {"methodVersion", "method-3"},
        {"vehicleId", "DJI-MAVIC-3E"},
        {"vehicleConfigurationVersion", "MAVIC-3E-STANDARD-REFERENCE-001"},
        {"equipmentProfileIds", QJsonArray{"DJI-MAVIC-3E-WIDE-PROFILE"}}
    });
    validatedOperationMission.insert("operations", QJsonArray{validatedOperation});
    const QVariantMap validatedOperationResult = bridge.calculate(
        QString::fromUtf8(QJsonDocument(validatedOperationMission).toJson(QJsonDocument::Compact)),
        validParameters());
    assert(validatedOperationResult.value("status").toString() == "COVERAGE_CANDIDATE_AVAILABLE");
    assert(validatedOperationResult.value("taskExecutionEstimatesReady").toBool());
    assert(!validatedOperationResult.value("taskExecutionEstimatesAppliedToTrajectory").toBool());

    QJsonObject mismatchedOperationMission = validatedOperationMission;
    QJsonObject mismatchedOperation = mismatchedOperationMission.value("operations").toArray().first().toObject();
    QJsonObject mismatchedEstimate = mismatchedOperation.value("executionEstimate").toObject();
    mismatchedEstimate.insert("vehicleConfigurationVersion", "OLD-CONFIGURATION");
    mismatchedOperation.insert("executionEstimate", mismatchedEstimate);
    mismatchedOperationMission.insert("operations", QJsonArray{mismatchedOperation});
    const QVariantMap rejectedEstimate = bridge.calculate(
        QString::fromUtf8(QJsonDocument(mismatchedOperationMission).toJson(QJsonDocument::Compact)),
        validParameters());
    assert(rejectedEstimate.value("status").toString() == "NEEDS_INPUT");
    assert(rejectedEstimate.value("findings").toStringList().contains(
        "MT01_OPERATION_ESTIMATE_VEHICLE_CONFIGURATION_MISMATCH"));

    QVariantMap mismatchParameters = validParameters();
    mismatchParameters.insert("cameraConfigurationId", "UNASSIGNED-CAMERA");
    const QVariantMap mismatch = bridge.calculate(validMissionJson(), mismatchParameters);
    assert(mismatch.value("status").toString() == "NEEDS_INPUT");
    assert(!mismatch.value("coverageCandidateAvailable").toBool());

    const QVariantMap malformed = bridge.calculate("{invalid", validParameters());
    assert(malformed.value("status").toString() == "NEEDS_INPUT");
    assert(malformed.value("findings").toStringList().contains("MISSION_JSON_INVALID"));

    QVariantMap missingInputs = validParameters();
    missingInputs.remove("cruiseSpeedMps");
    const QVariantMap missing = bridge.calculate(validMissionJson(), missingInputs);
    assert(missing.value("findings").toStringList().contains("MT01_NUMERIC_PARAMETERS_REQUIRED"));

    QJsonObject unassignedMissionObject = QJsonDocument::fromJson(validMissionJson().toUtf8()).object();
    unassignedMissionObject.insert("vehicleAssignments", QJsonArray{});
    unassignedMissionObject.insert("equipmentAssignments", QJsonArray{});
    const QVariantMap unassigned = bridge.calculate(
        QString::fromUtf8(QJsonDocument(unassignedMissionObject).toJson(QJsonDocument::Compact)),
        QVariantMap{});
    assert(unassigned.value("status").toString() == "NEEDS_INPUT");
    const QStringList unassignedFindings = unassigned.value("findings").toStringList();
    assert(unassignedFindings.contains("MT01_REQUIRES_EXACTLY_ONE_ASSIGNED_VEHICLE"));
    assert(unassignedFindings.contains("MT01_CAMERA_EQUIPMENT_ASSIGNMENT_REQUIRED"));
    assert(!unassigned.value("flightReleaseEligible").toBool());

    return 0;
}
