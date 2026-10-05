import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string missionId: ""
    property string missionSummary: ""
    property string missionReviewState: ""
    property var uavModel: []
    property int selectedUavIndex: -1

    // Operator inputs. The production planning core supplies/validates these values.
    property string areaOfInterest: "MAP AREA"
    property string deliverables: "POINT CLOUD · ORTHOMOSAIC · DSM/DTM · MESH"
    property string uavId: selectedUavIndex >= 0 && selectedUavIndex < uavModel.length
                            ? String(uavModel[selectedUavIndex].id) : "AUTO"
    property string payload: "AUTO"
    property string resolution: "AUTO"
    property string altitude: "AUTO"
    property string forwardOverlap: "AUTO"
    property string sideOverlap: "AUTO"
    property string rtkPpk: "AUTO"
    property string terrainFollowing: "AUTO"
    property string lineDirection: "AUTO"
    property string controlPoints: "OPTIONAL"
    property string qualityProfile: "STANDARD"

    readonly property color bg: "#07111E"
    readonly property color panel: "#0B1B2B"
    readonly property color line: "#174056"
    readonly property color cyan: "#00DDF2"
    readonly property color textColor: "#DCE8F2"
    readonly property color muted: "#91A8BA"
    readonly property color green: "#39D353"
    readonly property color warning: "#FFD339"

    signal closeRequested()
    signal applyRequested()

    function fieldValue(key) {
        return ({
            area: root.areaOfInterest,
            deliverables: root.deliverables,
            uav: root.uavId,
            payload: root.payload,
            resolution: root.resolution,
            altitude: root.altitude,
            forward: root.forwardOverlap,
            side: root.sideOverlap,
            rtk: root.rtkPpk,
            terrain: root.terrainFollowing,
            direction: root.lineDirection,
            points: root.controlPoints,
            quality: root.qualityProfile
        })[key]
    }

    Rectangle {
        anchors.fill: parent
        color: "#B8000710"
    }

    Rectangle {
        id: dialog
        anchors.centerIn: parent
        width: Math.min(1120, parent.width - 24)
        height: Math.min(760, parent.height - 20)
        color: root.bg
        border.color: root.cyan
        border.width: 1
        radius: 5

        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Rectangle {
                width: parent.width
                height: 38
                color: root.panel

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: "3D-КАРТОГРАФИЯ"
                    color: root.cyan
                    font.family: "B612"
                    font.pixelSize: 15
                    font.bold: true
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 150
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.missionId
                    color: root.muted
                    font.family: "B612 Mono"
                    font.pixelSize: 11
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: "INPUT → PLAN → VERIFY"
                    color: root.muted
                    font.family: "B612"
                    font.pixelSize: 10
                }
            }

            Row {
                width: parent.width
                height: parent.height - 92
                spacing: 8

                Rectangle {
                    width: parent.width * 0.55
                    height: parent.height
                    color: root.panel
                    border.color: root.line
                    border.width: 1

                    Flickable {
                        anchors.fill: parent
                        anchors.margins: 10
                        contentHeight: inputColumn.implicitHeight
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds

                        Column {
                            id: inputColumn
                            width: parent.width
                            spacing: 7

                            Text {
                                text: "ОПЕРАТОР"
                                color: root.cyan
                                font.family: "B612"
                                font.pixelSize: 11
                                font.bold: true
                            }

                            Repeater {
                                model: [
                                    ["AREA OF INTEREST", "area"],
                                    ["DELIVERABLES", "deliverables"],
                                    ["UAV", "uav"],
                                    ["PAYLOAD / SENSOR", "payload"],
                                    ["TARGET RESOLUTION / GSD", "resolution"],
                                    ["ALTITUDE", "altitude"],
                                    ["FORWARD OVERLAP", "forward"],
                                    ["SIDE OVERLAP", "side"],
                                    ["RTK / PPK", "rtk"],
                                    ["TERRAIN FOLLOWING", "terrain"],
                                    ["FLIGHT-LINE DIRECTION", "direction"],
                                    ["CONTROL / CHECK POINTS", "points"],
                                    ["QUALITY PROFILE", "quality"]
                                ]

                                delegate: Rectangle {
                                    required property var modelData
                                    width: inputColumn.width
                                    height: 34
                                    color: "#081522"
                                    border.color: root.line
                                    border.width: 1
                                    radius: 2

                                    Text {
                                        x: 9
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width * 0.46
                                        text: modelData[0]
                                        color: root.muted
                                        font.family: "B612"
                                        font.pixelSize: 9
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }

                                    TextField {
                                        id: editor
                                        x: parent.width * 0.46
                                        width: parent.width * 0.54 - 6
                                        height: parent.height - 4
                                        y: 2
                                        text: root.fieldValue(modelData[1])
                                        color: root.textColor
                                        placeholderText: "AUTO"
                                        font.family: "B612"
                                        font.pixelSize: 11
                                        background: Rectangle { color: "transparent" }
                                        onEditingFinished: {
                                            if (modelData[1] === "resolution") root.resolution = text
                                            else if (modelData[1] === "altitude") root.altitude = text
                                            else if (modelData[1] === "forward") root.forwardOverlap = text
                                            else if (modelData[1] === "side") root.sideOverlap = text
                                            else if (modelData[1] === "payload") root.payload = text
                                            else if (modelData[1] === "rtk") root.rtkPpk = text
                                            else if (modelData[1] === "terrain") root.terrainFollowing = text
                                            else if (modelData[1] === "direction") root.lineDirection = text
                                            else if (modelData[1] === "points") root.controlPoints = text
                                            else if (modelData[1] === "quality") root.qualityProfile = text
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width * 0.45 - 8
                    height: parent.height
                    color: root.panel
                    border.color: root.line
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        Text {
                            text: "BLUE SKY — РАСЧЁТ"
                            color: root.cyan
                            font.family: "B612"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        Repeater {
                            model: [
                                ["FLIGHT-LINE SPACING", "—"],
                                ["NUMBER OF LINES", "—"],
                                ["EXPECTED FRAMES", "—"],
                                ["EXPECTED COVERAGE", "—"],
                                ["ROUTE LENGTH", "—"],
                                ["ESTIMATED DURATION", "—"],
                                ["ENERGY / RESERVE", "—"],
                                ["DATA VOLUME", "—"]
                            ]

                            delegate: Rectangle {
                                required property var modelData
                                width: parent.width
                                height: 31
                                color: "#081522"
                                border.color: root.line
                                border.width: 1
                                radius: 2

                                Text {
                                    x: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width * 0.62
                                    text: modelData[0]
                                    color: root.muted
                                    font.family: "B612"
                                    font.pixelSize: 9
                                    font.bold: true
                                    elide: Text.ElideRight
                                }
                                Text {
                                    anchors.right: parent.right
                                    anchors.rightMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData[1]
                                    color: root.textColor
                                    font.family: "B612 Mono"
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }
                        }

                        Rectangle {
                            width: parent.width
                            height: 1
                            color: root.line
                        }

                        Text {
                            text: "VALIDATION"
                            color: root.cyan
                            font.family: "B612"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        Text {
                            width: parent.width
                            text: "AREA · UAV/PAYLOAD · ACQUISITION GEOMETRY ·\nCONSTRAINTS · COVERAGE · ENERGY · DATA QUALITY"
                            color: root.muted
                            font.family: "B612"
                            font.pixelSize: 10
                            lineHeight: 1.2
                        }

                        Rectangle {
                            width: parent.width
                            height: 34
                            color: "#10251A"
                            border.color: root.green
                            border.width: 1
                            radius: 2

                            Text {
                                anchors.fill: parent
                                text: "WAITING FOR PLANNING CORE"
                                color: root.green
                                font.family: "B612"
                                font.pixelSize: 10
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Item { width: 1; height: 1 }
                    }
                }
            }

            Row {
                width: parent.width
                height: 38
                spacing: 8

                Rectangle {
                    width: parent.width - 168
                    height: parent.height
                    color: "transparent"

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Ручные изменения → новый расчёт → повторная проверка"
                        color: root.warning
                        font.family: "B612"
                        font.pixelSize: 10
                    }
                }

                Rectangle {
                    width: 76
                    height: parent.height
                    color: "transparent"
                    border.color: root.line
                    border.width: 1
                    radius: 2
                    Text {
                        anchors.fill: parent
                        text: "ЗАКРЫТЬ"
                        color: root.textColor
                        font.family: "B612"
                        font.pixelSize: 10
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea { anchors.fill: parent; onClicked: root.closeRequested() }
                }

                Rectangle {
                    width: 84
                    height: parent.height
                    color: root.cyan
                    radius: 2
                    Text {
                        anchors.fill: parent
                        text: "ПРИМЕНИТЬ"
                        color: "#041018"
                        font.family: "B612"
                        font.pixelSize: 10
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    MouseArea { anchors.fill: parent; onClicked: root.applyRequested() }
                }
            }
        }
    }
}
