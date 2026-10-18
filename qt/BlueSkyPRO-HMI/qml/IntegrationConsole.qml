import QtQuick
import QtQuick.Controls

Item {
    id: root
    property bool testStubsEnabled: true
    property int selectedIntegration: 0
    readonly property color panel: "#0B1724"
    readonly property color line: "#26394B"
    readonly property color text: "#E8EEF5"
    readonly property color muted: "#8EA2B8"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#FFD166"

    function integrationName(index) {
        return ["NOTAM / AIP", "WX / METEO", "AIRSPACE", "TERRAIN / DEM",
                "OBSTACLES", "TRAFFIC", "ATM / FPL", "AUTHORIZATION", "GIS / MAP DATA"][index]
    }
    function integrationDescription(index) {
        return [
            "NOTAM messages, effective windows, cancellation and replacement chain",
            "METAR, SPECI, TAF, GAMET, SIGWX, SIGMET/AIRMET and wind fields",
            "Restricted/prohibited areas, temporary regimes and altitude limits",
            "Terrain elevation model, vertical datum and coverage",
            "Obstacle database, source date, position and elevation reference",
            "Traffic data and availability of the applicable service",
            "Flight-plan exchange channel, acknowledgements and external references",
            "Permission status, scope, conditions, validity and mission-version link",
            "Map/geospatial provider, coordinate reference and dataset version"
        ][index]
    }

    Rectangle { anchors.fill: parent; color: "transparent" }

    Column {
        anchors.fill: parent
        anchors.margins: 24
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
                MouseArea { anchors.fill: parent; onClicked: root.testStubsEnabled = !root.testStubsEnabled }
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
                model: 9
                delegate: Rectangle {
                    width: (root.width - 48 - 20) / 3
                    height: 116
                    radius: 3
                    color: root.selectedIntegration === index ? "#10283A" : root.panel
                    border.color: root.selectedIntegration === index ? root.cyan : root.line
                    Column {
                        anchors.fill: parent; anchors.margins: 12; spacing: 7
                        Row {
                            width: parent.width
                            Text { width: parent.width - 64; text: root.integrationName(index); color: root.text; font.family: "B612"; font.pixelSize: 12; font.bold: true; elide: Text.ElideRight }
                            Text { text: "STUB"; color: root.testStubsEnabled ? root.amber : root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                        }
                        Text { width: parent.width; height: 42; text: root.integrationDescription(index); color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap; elide: Text.ElideRight }
                        Text { text: root.testStubsEnabled ? "LIVE: DISCONNECTED  /  TEST: AVAILABLE" : "LIVE: DISCONNECTED  /  TEST: OFF"; color: root.testStubsEnabled ? root.amber : root.muted; font.family: "B612 Mono"; font.pixelSize: 8 }
                    }
                    MouseArea { anchors.fill: parent; onClicked: root.selectedIntegration = index }
                }
            }
        }

        Rectangle { width: parent.width; height: 1; color: root.line }

        Column {
            width: parent.width
            spacing: 7
            Text { text: "SELECTED CONNECTOR  /  " + root.integrationName(root.selectedIntegration); color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
            Text { width: parent.width; text: root.integrationDescription(root.selectedIntegration); color: root.text; font.family: "B612"; font.pixelSize: 11; wrapMode: Text.Wrap }
            Text { width: parent.width; text: "Connection endpoint, credentials, licence, protocol, coverage, update cadence and freshness limits are not configured. This screen does not establish a network connection."; color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
        }
    }
}
