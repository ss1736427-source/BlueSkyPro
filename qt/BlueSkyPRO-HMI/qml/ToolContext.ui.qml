import QtQuick

Item {
    id: root

    property string contextName: "UAV"
    property string contextSubtitle: ""
    property var sections: []
    property int selectedUavIndex: -1
    property string selectedUavId: "NO UAV SELECTED"
    property var planningResult: null
    property string simulationRunnerPath: ""
    property bool simulationRunning: false
    property string simulationError: ""
    readonly property bool hasSimulationResult: root.planningResult && root.planningResult.simulationAcceptance === "PASS"
    signal runSimulationRequested()

    Rectangle {
        anchors.fill: parent
        color: "transparent"
    }

    Text {
        x: 24
        y: 22
        text: root.contextName === "UAV" ? "UAV  /  " + root.selectedUavId : root.contextName
        color: "#FFFFFF"
        font.family: "B612"
        font.pixelSize: 20
        font.bold: true
    }

    Text {
        x: 24
        y: 52
        text: root.contextSubtitle
        color: "#7F7F7F"
        font.family: "B612 Mono"
        font.pixelSize: 10
    }

    Rectangle {
        x: 24
        y: 82
        width: parent.width - 48
        height: 1
        color: "#202020"
    }

    Rectangle {
        visible: root.contextName === "FPV"
        x: 24
        y: 104
        width: parent.width - 48
        height: 250
        color: "#000000"
        border.color: "#202020"
        border.width: 1

        Text {
            x: 18
            y: 14
            text: root.selectedUavId + "   CONTROL STATE: UNAVAILABLE"
            color: "#FFD339"
            font.family: "B612 Mono"
            font.pixelSize: 13
            font.bold: true
        }

        Rectangle {
            x: 18
            y: 44
            width: parent.width - 36
            height: 118
            color: "#08111D"
            border.color: "#202020"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "CAMERA VIDEO"
                color: "#7F7F7F"
                font.family: "B612 Mono"
                font.pixelSize: 12
            }
        }

        Text { x: 18; y: 174; text: "TELEMETRY: NOT CONNECTED"; color: "#FFD339"; font.family: "B612 Mono"; font.pixelSize: 10 }
        Text { x: 18; y: 196; text: "C2: UNKNOWN     VIDEO: UNKNOWN     RC: UNKNOWN"; color: "#BFBFBF"; font.family: "B612 Mono"; font.pixelSize: 10 }

        Rectangle {
            x: parent.width - 190
            y: 180
            width: 172
            height: 42
            color: "transparent"
            border.color: "#FFD339"
            border.width: 1

            Text {
                anchors.fill: parent
                text: "CONTROL TRANSFER UNAVAILABLE"
                color: "#FFD339"
                font.family: "B612 Mono"
                font.pixelSize: 10
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

        }

        Text {
            x: 18
            y: 226
            text: "Prototype display only — no control link"
            color: "#7F7F7F"
            font.family: "B612"
            font.pixelSize: 10
        }
    }


    Rectangle {
        id: virtualFlightPanel
        visible: root.contextName === "VIRTUAL FLT"
        x: 24
        y: 104
        width: Math.max(360, parent.width - 48)
        height: 246
        color: "#08111D"
        border.color: "#24384B"
        border.width: 1

        Text {
            x: 14
            y: 12
            text: "VIRTUAL FLIGHT / REPRESENTATIVE SIMULATION"
            color: "#FFFFFF"
            font.family: "B612"
            font.pixelSize: 14
            font.bold: true
        }

        Rectangle {
            x: parent.width - 190
            y: 10
            width: 174
            height: 34
            color: root.simulationRunning ? "#24384B" : "#64FF00"
            radius: 2
            Text {
                anchors.centerIn: parent
                text: root.simulationRunning ? "SIMULATION RUNNING" : (root.hasSimulationResult ? "RUN AGAIN" : "RUN SIMULATION")
                color: root.simulationRunning ? "#FFFFFF" : "#07100A"
                font.family: "B612 Mono"
                font.pixelSize: 10
                font.bold: true
            }
            MouseArea {
                anchors.fill: parent
                enabled: !root.simulationRunning
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: root.runSimulationRequested()
            }
        }

        Text {
            x: 14
            y: 42
            width: parent.width - 28
            text: root.planningResult && root.planningResult.uav
                  ? root.planningResult.uav.name + " / " + root.planningResult.uav.camera
                  : "DJI Mavic 3 Enterprise / 4/3 CMOS / 5280x3956 / mechanical shutter"
            color: "#32FFFF"
            font.family: "B612 Mono"
            font.pixelSize: 10
            elide: Text.ElideRight
        }

        Rectangle { x: 14; y: 66; width: parent.width - 28; height: 1; color: "#24384B" }

        Row {
            x: 14
            y: 78
            width: parent.width - 28
            spacing: 24
            Column {
                width: 210
                spacing: 5
                Text { text: "WEATHER / SYNTHETIC"; color: "#7F7F7F"; font.family: "B612 Mono"; font.pixelSize: 9 }
                Text {
                    text: root.planningResult && root.planningResult.environment
                          ? Number(root.planningResult.environment.temperatureC).toFixed(0) + " C / E "
                            + Number(root.planningResult.environment.windEastMps).toFixed(1) + " m/s / precip "
                            + Number(root.planningResult.environment.precipitationMmPerHour).toFixed(1) + " mm/h"
                          : "15 C / E 1.5 m/s / precip 0.0 mm/h"
                    color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 11
                }
            }
            Column {
                width: 280
                spacing: 5
                Text { text: "NOTAM / RESTRICTIONS"; color: "#7F7F7F"; font.family: "B612 Mono"; font.pixelSize: 9 }
                Text {
                    text: root.planningResult && root.planningResult.environment
                          ? root.planningResult.environment.notamId + " / SIMULATED, NOT CLEARANCE"
                          : "SIM-NOTAM-001 / SYNTHETIC, NOT CLEARANCE"
                    color: "#FFD339"; font.family: "B612 Mono"; font.pixelSize: 10
                }
            }
            Column {
                width: 220
                spacing: 5
                Text { text: "SCENARIO / DATASET"; color: "#7F7F7F"; font.family: "B612 Mono"; font.pixelSize: 9 }
                Text { text: "V-M01-01-SIM-001"; color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 11 }
            }
        }

        Rectangle { x: 14; y: 128; width: parent.width - 28; height: 1; color: "#24384B" }

        Row {
            x: 14
            y: 140
            width: parent.width - 28
            spacing: 22
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "GSD " + Number(root.planningResult.metrics.gsdWidthMPerPx).toFixed(3) + " m/px" : "GSD —"
                color: "#32FFFF"; font.family: "B612 Mono"; font.pixelSize: 11
            }
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "COVERAGE " + (Number(root.planningResult.metrics.coverageRatio) * 100).toFixed(1) + "%" : "COVERAGE —"
                color: "#64FF00"; font.family: "B612 Mono"; font.pixelSize: 11
            }
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "TRACKS " + root.planningResult.metrics.coverageTracks : "TRACKS —"
                color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 11
            }
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "ENERGY " + Number(root.planningResult.metrics.selectedRouteEnergyWh).toFixed(2) + " Wh" : "ENERGY —"
                color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 11
            }
        }

        Row {
            x: 14
            y: 164
            width: parent.width - 28
            spacing: 22
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "TIME " + Number(root.planningResult.metrics.selectedRouteTimeS).toFixed(2) + " s / MODEL"
                      : "TIME — / MODEL"
                color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 10
            }
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "TRIGGER " + Number(root.planningResult.metrics.triggerIntervalS).toFixed(2) + " s"
                      : "TRIGGER —"
                color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 10
            }
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "EVENTS " + root.planningResult.metrics.acquisitionEvents
                      : "EVENTS —"
                color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 10
            }
            Text {
                text: root.planningResult && root.planningResult.metrics
                      ? "EDGE GAPS " + root.planningResult.metrics.edgeGaps
                      : "EDGE GAPS —"
                color: "#FFFFFF"; font.family: "B612 Mono"; font.pixelSize: 10
            }
        }

        Text {
            x: 14
            y: 201
            width: parent.width - 28
            text: root.simulationError.length > 0
                  ? "RUNNER ERROR: " + root.simulationError
                  : root.hasSimulationResult
                    ? "SIMULATION PASS / RELEASE BLOCKED / " + root.planningResult.operationalStatus
                    : "READY TO RUN / MAP ROUTE WILL COME FROM THE PLANNING ENGINE"
            color: root.simulationError.length > 0 ? "#FF4D5A" : "#FFD339"
            font.family: "B612 Mono"
            font.pixelSize: 10
            elide: Text.ElideRight
        }
    }

    Flow {
        visible: root.contextName !== "FPV" && root.contextName !== "VIRTUAL FLT"
        x: 24
        y: 104
        width: parent.width - 48
        spacing: 10

        Repeater {
            model: root.sections

            delegate: Rectangle {
                width: 220
                height: 74
                color: "#08111D"
                border.color: "#202020"
                border.width: 1

                Text {
                    x: 12
                    y: 12
                    text: modelData
                    color: "#BFBFBF"
                    font.family: "B612"
                    font.pixelSize: 12
                    font.bold: true
                }

                Text {
                    x: 12
                    y: 38
                    text: "CONFIGURABLE"
                    color: "#7F7F7F"
                    font.family: "B612 Mono"
                    font.pixelSize: 9
                }
            }
        }
    }

    PanelSettingsButton {
        id: settings
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 14
        onClicked: popup.open = !popup.open
    }

    PanelSettingsPopup {
        id: popup
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 48
        width: 280
        height: Math.min(420, 80 + root.sections.length * 28)
        title: root.contextName + " SETTINGS"
        tools: root.sections
        onClosed: open = false
    }
}
