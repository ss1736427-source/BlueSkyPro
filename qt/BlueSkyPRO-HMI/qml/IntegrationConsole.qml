import QtQuick
import QtQuick.Controls

Item {
    id: root
    property bool testStubsEnabled: true
    property int selectedIntegration: 0
    signal testStubsEnabledRequested(bool enabled)
    readonly property color panel: "#0B1724"
    readonly property color line: "#26394B"
    readonly property color text: "#E8EEF5"
    readonly property color muted: "#8EA2B8"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#FFD166"

    function integrationName(index) {
        return [
            "NOTAM / AIP", "WX / METEO", "AIRSPACE / RESTRICTIONS",
            "TERRAIN / DEM", "OBSTACLES", "TRAFFIC / UTM",
            "ATM / FPL / ЕС ОрВД", "AUTHORIZATION / PERMITS", "GIS / MAPTILER",
            "GNSS / RTK / PPK", "UAV / FCS / MAVLINK", "C2 / DATA LINKS",
            "PAYLOAD / CAMERA / VIDEO", "IDENTITY / ACCESS / SECURITY",
            "INSURANCE", "FLIGHT RECORD / LOGS", "SYNC / EXTERNAL API",
            "ALERTS / OPERATIONAL EVENTS"
        ][index]
    }
    function integrationDescription(index) {
        return [
            "NOTAM messages, validity windows, cancellation and replacement chain",
            "METAR, SPECI, TAF, GAMET, SIGWX, SIGMET/AIRMET, wind and hazards",
            "Controlled, restricted and prohibited areas, temporary regimes, altitude limits",
            "Terrain elevation model, vertical datum, resolution and coverage",
            "Obstacle positions, elevations, source date and reference datum",
            "Traffic information, UTM services and availability state",
            "Flight-plan exchange, acknowledgements, status and external references",
            "Permission identity, scope, conditions, validity and mission-version binding",
            "Map tiles/styles, geospatial services, coordinate reference and dataset version",
            "GNSS corrections, RTK/PPK service, correction age and quality",
            "Autopilot/FCS protocols, MAVLink, capability and firmware compatibility",
            "C2 transports, link quality, latency, failover and recovery",
            "Payload control, camera/sensor SDKs, video and metadata",
            "Operator/device identity, authentication, roles, credentials and audit",
            "Insurance provider interface, policy scope, status and evidence",
            "Flight Record, logs, replay, provenance and retention",
            "Offline queue, synchronization, conflict resolution and external APIs",
            "System warnings, external-service failures and change notifications"
        ][index]
    }

    Rectangle { anchors.fill: parent; color: "transparent" }

    Flickable {
        id: flick
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentColumn.implicitHeight + 48
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentColumn
            x: 24
            y: 24
            width: flick.width - 48
            spacing: 14

            Row {
                width: parent.width
                spacing: 14
                Column {
                    width: parent.width - 250
                    spacing: 5
                    Text { text: "EXTERNAL INTEGRATIONS"; color: root.text; font.family: "B612"; font.pixelSize: 22; font.bold: true }
                    Text { text: "ADMINISTRATOR / CONNECTORS / DATA QUALITY"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 10 }
                }
                Rectangle {
                    width: 220; height: 38; radius: 3
                    color: root.testStubsEnabled ? "#332A1B" : "#182330"
                    border.color: root.testStubsEnabled ? root.amber : root.line
                    Text {
                        anchors.centerIn: parent
                        text: root.testStubsEnabled ? "TEST STUBS: ENABLED" : "TEST STUBS: DISABLED"
                        color: root.testStubsEnabled ? root.amber : root.muted
                        font.family: "B612 Mono"; font.pixelSize: 10; font.bold: true
                    }
                    MouseArea { anchors.fill: parent; onClicked: root.testStubsEnabledRequested(!root.testStubsEnabled) }
                }
            }

            Rectangle {
                width: parent.width; height: 62; radius: 3
                color: "#2B2115"; border.color: "#8B682F"
                Column {
                    anchors.fill: parent; anchors.margins: 10; spacing: 4
                    Text { text: "NON-OPERATIONAL TEST CONTOUR"; color: root.amber; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                    Text { text: "No live provider is connected. Test fixtures must never be used to authorize or release a flight."; color: root.text; font.family: "B612"; font.pixelSize: 11; wrapMode: Text.Wrap; width: parent.width }
                }
            }

            Grid {
                width: parent.width
                columns: 3
                spacing: 10
                Repeater {
                    model: 18
                    delegate: Rectangle {
                        width: (root.width - 48 - 20) / 3
                        height: 102
                        radius: 3
                        color: root.selectedIntegration === index ? "#10283A" : root.panel
                        border.color: root.selectedIntegration === index ? root.cyan : root.line
                        Column {
                            anchors.fill: parent; anchors.margins: 10; spacing: 6
                            Row {
                                width: parent.width
                                Text { width: parent.width - 40; text: root.integrationName(index); color: root.text; font.family: "B612"; font.pixelSize: 10; font.bold: true; elide: Text.ElideRight }
                                Text { text: "STUB"; color: root.testStubsEnabled ? root.amber : root.muted; font.family: "B612 Mono"; font.pixelSize: 8 }
                            }
                            Text { width: parent.width; height: 42; text: root.integrationDescription(index); color: root.muted; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap; elide: Text.ElideRight }
                            Text { text: root.testStubsEnabled ? "LIVE: DISCONNECTED / TEST: AVAILABLE" : "LIVE: DISCONNECTED / TEST: OFF"; color: root.testStubsEnabled ? root.amber : root.muted; font.family: "B612 Mono"; font.pixelSize: 7 }
                        }
                        MouseArea { anchors.fill: parent; onClicked: root.selectedIntegration = index }
                    }
                }
            }

            Rectangle { width: parent.width; height: 1; color: root.line }

            Column {
                width: parent.width
                spacing: 7
                Text { text: "SELECTED CONNECTOR / " + root.integrationName(root.selectedIntegration); color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                Text { width: parent.width; text: root.integrationDescription(root.selectedIntegration); color: root.text; font.family: "B612"; font.pixelSize: 11; wrapMode: Text.Wrap }
                Text { width: parent.width; text: "Endpoint, credentials, licence, protocol, coverage, update cadence and freshness limits are not configured. Selecting a card only inspects its contract; this screen does not establish a network connection."; color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
            }
        }
    }
}
