import QtQuick
import QtQuick.Controls

Item {
    id: root
    property int scenarioIndex: 0
    property bool testStubsEnabled: true
    property bool runRequested: false
    property var assessmentResult: ({})
    property string manualGeometryJson: "{\\"objects\\":[]}"
    property var manualTemplateIds: []
    property string manualMissionId: "VF-MANUAL-MISSION"
    property int manualMissionVersion: 1
    property bool manualCompositionComplete: false
    readonly property color panel: "#0B1724"
    readonly property color line: "#26394B"
    readonly property color text: "#E8EEF5"
    readonly property color muted: "#8EA2B8"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#FFD166"

    // Versioned non-operational fixture contract. These are synthetic values,
    // not observations or a sourced historical climatology dataset.
    readonly property var scenarios: [
        {
            profileId: "VF-WX-SYN-001",
            sourceType: "SYNTHETIC_AVERAGE",
            datasetVersion: "VF-FIXTURES-1.0",
            locationId: "UNSPECIFIED",
            averagingPeriod: "UNSPECIFIED",
            name: "SYNTHETIC AVERAGE / CALM",
            windMeanMps: 3.0, windGustMps: 5.0, temperatureC: 15,
            visibilityKm: 10, cloudAmount: "FEW", cloudBaseM: 1500,
            syntheticNotamRecordCount: 2,
            profile: "Synthetic average fixture — no real station, location, date range or historical source"
        },
        {
            profileId: "VF-WX-SYN-002",
            sourceType: "SYNTHETIC_AVERAGE",
            datasetVersion: "VF-FIXTURES-1.0",
            locationId: "UNSPECIFIED",
            averagingPeriod: "UNSPECIFIED",
            name: "SYNTHETIC AVERAGE / WINDY",
            windMeanMps: 8.0, windGustMps: 12.0, temperatureC: 9,
            visibilityKm: 8, cloudAmount: "BKN", cloudBaseM: 900,
            syntheticNotamRecordCount: 4,
            profile: "Synthetic average fixture — no real station, location, date range or historical source"
        },
        {
            profileId: "VF-WX-SYN-003",
            sourceType: "SYNTHETIC_AVERAGE",
            datasetVersion: "VF-FIXTURES-1.0",
            locationId: "UNSPECIFIED",
            averagingPeriod: "UNSPECIFIED",
            name: "SYNTHETIC AVERAGE / LOW VIS",
            windMeanMps: 4.0, windGustMps: 7.0, temperatureC: 6,
            visibilityKm: 3, cloudAmount: "OVC", cloudBaseM: 450,
            syntheticNotamRecordCount: 3,
            profile: "Synthetic average fixture — no real station, location, date range or historical source"
        }
    ]
    readonly property var scenario: scenarios[scenarioIndex]

    Rectangle { anchors.fill: parent; color: "transparent" }

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 14

        Column {
            spacing: 5
            Text { text: "VIRTUAL FLIGHT / TEST ENVIRONMENT"; color: root.text; font.family: "B612"; font.pixelSize: 22; font.bold: true }
            Text { text: "SIMULATION INPUTS / MOCK CONNECTORS / REPLAY"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 10 }
        }

        Rectangle {
            width: parent.width; height: 64; radius: 3
            color: "#2B2115"; border.color: "#8B682F"
            Column {
                anchors.fill: parent; anchors.margins: 10; spacing: 4
                Text { text: root.testStubsEnabled ? "SYNTHETIC TEST DATA — NOT FOR FLIGHT OPERATIONS" : "TEST STUBS DISABLED — NO PROFILE INPUT"; color: root.amber; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                Text { text: root.testStubsEnabled ? "Weather values are illustrative averages for exercising UI and planning interfaces; NOTAM records and zones are stubs, not current aeronautical information." : "Enable test stubs in ADMIN / EXTERNAL INTEGRATIONS before selecting a fixture. No live provider is connected."; color: root.text; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap; width: parent.width }
            }
        }

        Row {
            width: parent.width; height: 42; spacing: 10
            Repeater {
                model: root.scenarios
                delegate: Rectangle {
                    width: (root.width - 48 - 20) / 3
                    height: 42
                    radius: 3
                    color: root.scenarioIndex === index ? "#10283A" : root.panel
                    border.color: root.scenarioIndex === index ? root.cyan : root.line
                    Text { anchors.centerIn: parent; text: modelData.name; color: root.scenarioIndex === index ? root.cyan : root.text; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: { if (root.testStubsEnabled) { root.scenarioIndex = index; root.runRequested = false } } }
                }
            }
        }

        Grid {
            width: parent.width
            columns: 3
            spacing: 10
            Repeater {
                model: [
                    { label: "WIND / MEAN", value: root.scenario.windMeanMps.toFixed(1) + " m/s" },
                    { label: "WIND / GUST", value: root.scenario.windGustMps.toFixed(1) + " m/s" },
                    { label: "AIR TEMPERATURE", value: root.scenario.temperatureC + " °C" },
                    { label: "VISIBILITY", value: root.scenario.visibilityKm + " km" },
                    { label: "CLOUD BASE", value: root.scenario.cloudAmount + " / " + root.scenario.cloudBaseM + " m" },
                    { label: "NOTAM STUBS", value: root.scenario.syntheticNotamRecordCount + " synthetic test records" }
                ]
                delegate: Rectangle {
                    width: (root.width - 48 - 20) / 3
                    height: 76
                    radius: 3
                    color: root.panel
                    border.color: root.line
                    Column {
                        anchors.fill: parent; anchors.margins: 12; spacing: 8
                        Text { text: modelData.label; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                        Text { text: modelData.value; color: root.text; font.family: "B612"; font.pixelSize: 16; font.bold: true }
                    }
                }
            }
        }

        Rectangle {
            width: parent.width; height: 62; radius: 3
            color: root.panel; border.color: root.line
            Column {
                anchors.fill: parent; anchors.margins: 10; spacing: 5
                Text { text: "DATA PROVENANCE"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 10; font.bold: true }
                Text { width: parent.width; text: "PROFILE " + root.scenario.profileId + " / " + root.scenario.sourceType + " / " + root.scenario.datasetVersion + " / LOCATION " + root.scenario.locationId + " / PERIOD " + root.scenario.averagingPeriod + " — " + root.scenario.profile; color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
            }
        }

        Rectangle {
            visible: root.runRequested
            width: parent.width; height: 108; radius: 3
            color: "#0B1724"; border.color: root.assessmentResult.combinedClear ? "#8B682F" : "#8B3A3A"
            Column {
                anchors.fill: parent; anchors.margins: 10; spacing: 4
                Text { text: "CANONICAL CORE RESULT / " + String(root.assessmentResult.profileId || "NO PROFILE"); color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 10; font.bold: true }
                Text { width: parent.width; text: "ROUTE: " + String(root.assessmentResult.routeSource || "UNAVAILABLE") + " / NOTAM: " + String(root.assessmentResult.notamRecordCount || 0) + " / FIXTURE: " + String(root.assessmentResult.datasetVersion || "UNKNOWN") + " / RELEASE ELIGIBLE: NO"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 9; wrapMode: Text.Wrap }
                Text { width: parent.width; text: (root.assessmentResult.errors || []).join(" · ") || "Airspace/NOTAM clear only; all other operational gates remain unchecked."; color: root.assessmentResult.combinedClear ? root.amber : "#FF8C8C"; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                Text { width: parent.width; text: String(root.assessmentResult.scope || "Scope unavailable") ; color: root.muted; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
            }
        }

        Row {
            spacing: 12
            Rectangle {
                width: 190; height: 42; radius: 3
                color: "#10283A"; border.color: root.cyan
                Text { anchors.centerIn: parent; text: !root.testStubsEnabled ? "TEST STUBS OFF" : (root.runRequested ? "ASSESSMENT COMPLETE" : "RUN CORE ASSESSMENT"); color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 10; font.bold: true }
                MouseArea { anchors.fill: parent; onClicked: { if (root.testStubsEnabled) { root.assessmentResult = root.manualCompositionComplete && root.manualTemplateIds.length > 0 && JSON.parse(root.manualGeometryJson).objects.length > 0 ? virtualFlightAssessmentBridge.assessManualGeometry(root.manualGeometryJson, root.manualTemplateIds, root.manualMissionId, root.manualMissionVersion, root.manualCompositionComplete, root.scenarioIndex) : virtualFlightAssessmentBridge.assessSyntheticFixture(root.scenarioIndex); root.runRequested = true } } }
            }
            Text {
                width: root.width - 260
                text: !root.testStubsEnabled
                      ? "Test fixtures are disabled in Administrator. No profile is available for this session."
                      : (root.runRequested
                         ? "Canonical core assessment completed. See result panel. Flight release is always blocked."
                         : "Run the canonical planning-core assessment against a fixed synthetic route and synthetic NOTAM snapshot. This does not use the operator map route or authorize a flight.")
                color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap; verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
