#include "Mt01MappingBridge.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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
        {"selectedTemplateIds", QJsonArray{"MT-01"}},
        {"task", QJsonObject{{"description", "Photogrammetry test"}}},
        {"intent", QJsonObject{{"objective", "Orthomosaic"}, {"taskType", "MAPPING"}}},
        {"vehicleAssignments", QJsonArray{QJsonObject{
            {"vehicleId", "DJI-MAVIC-3E"},
            {"configurationId", "MAVIC-3E-STANDARD"},
            {"configurationVersion", "MAVIC-3E-STANDARD-REFERENCE-001"},
            {"configurationType", "STANDARD"}
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
        {"geometryObjects", QJsonArray{aoi}}
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
        {"intrinsicCalibrationVerified", false}
    };
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Mt01MappingBridge bridge;

    const QVariantMap result = bridge.calculate(validMissionJson(), validParameters());
    assert(result.value("status").toString() == "COVERAGE_CANDIDATE_AVAILABLE");
    assert(result.value("coverageCandidateAvailable").toBool());
    assert(!result.value("flightReleaseEligible").toBool());
    assert(result.value("coverageValid").toBool());
    assert(result.value("waypoints").toList().size() >= 2);
    assert(!result.value("capturePoints").toList().isEmpty());
    const QVariantMap metrics = result.value("metrics").toMap();
    assert(metrics.value("coverageLineCount").toULongLong() > 0);
    assert(metrics.value("calculatedAltitudeAglM").toDouble() > 100.0);

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
