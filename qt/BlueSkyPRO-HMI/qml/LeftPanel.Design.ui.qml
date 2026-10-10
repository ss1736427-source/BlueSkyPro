import QtQuick

Item {
    id: root
    width: 430
    height: 720

    property color bg: "#08111D"
    property color card: "#0C1725"
    property color selectedSurface: "#111F30"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color muted: "#7F7F7F"
    property color cyan: "#32FFFF"
    property color divider: "#7F7F7F"

    property string missionState: "AUTO"
    readonly property bool missionVisible: missionState === "AUTO"
    readonly property bool missionCreationMode: missionState === "MANUAL"
    property bool missionIdExpanded: false
    property string missionReviewState: "REWORK"
    readonly property color missionIdStatusColor: missionReviewState === "VERIFIED" ? "#64FF00" : "#FF00FF"
    property bool panelConfigOpen: false
    property int selectedTemplate: 1
    property bool analysisVisible: true
    property bool templatesExpanded: true
    property bool instrumentsVisible: false
    property bool atcVisible: false
    property bool diagnosticsVisible: false

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    // The neighboring header owns this shared top seam; draw it once here
    // as the seam reference, not as a second panel outline.
    Rectangle {
        x: 0
        y: 0
        width: parent.width
        height: 1
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        x: 0
        y: 1
        width: 1
        height: parent.height - 1
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        x: parent.width - 1
        y: 1
        width: 1
        height: parent.height - 1
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        x: 0
        y: parent.height - 1
        width: parent.width
        height: 1
        color: root.cyan
        antialiasing: false
    }

    // Panel title bar: selected surface, cyan outline, controlled typography.
    Rectangle {
        x: 10
        y: 8
        width: parent.width - 20
        height: 38
        color: "transparent"
        border.width: 0

        Text {
            x: 10
            anchors.verticalCenter: parent.verticalCenter
            text: "Миссии"
            color: root.text
            font.family: "B612"
            font.pixelSize: 13
            font.bold: true
        }

        Text {
            x: parent.width - 58
            anchors.verticalCenter: parent.verticalCenter
            text: "+"
            color: root.missionVisible ? root.cyan : root.text
            font.family: "B612 Mono"
            font.pixelSize: 16
            font.bold: true

            MouseArea {
                anchors.fill: parent
                onClicked: root.missionState = "AUTO"
            }
        }

        Text {
            x: parent.width - 28
            anchors.verticalCenter: parent.verticalCenter
            text: "≡"
            color: root.panelConfigOpen ? root.text : root.cyan
            font.family: "B612 Mono"
            font.pixelSize: 16
        }

        MouseArea {
            x: parent.width - 44
            width: 34
            height: parent.height
            onClicked: root.panelConfigOpen = !root.panelConfigOpen
        }
    }

    Rectangle {
        visible: root.missionVisible
        x: 10
        y: 50
        width: parent.width - 20
        height: 40
        radius: 3
        color: root.card
        border.color: root.divider
        border.width: 1

        Text {
            id: missionIdLabel
            x: 10
            width: Math.min(implicitWidth, Math.max(0, parent.width - x - 110))
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideNone
            text: "BS-260920-A-001"
            fontSizeMode: Text.Fit
            minimumPixelSize: 8
            wrapMode: Text.NoWrap
            color: root.missionIdStatusColor
            font.family: "B612 Mono"
            font.pixelSize: 11
            font.bold: true

            MouseArea {
                anchors.fill: parent
                onClicked: root.missionIdExpanded = !root.missionIdExpanded
            }
        }

        Rectangle {
            x: missionIdLabel.x + missionIdLabel.width + 5
            y: 8
            width: 1
            height: parent.height - 16
            color: root.divider
        }

        Text {
            id: missionSummaryLabel
            x: missionIdLabel.x + missionIdLabel.width + 17
            width: Math.max(0, parent.width - x - 12)
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            text: "3D-картография территории"
            color: root.secondary
            font.family: "Noto Sans"
            font.pixelSize: 10
        }

    }

    Rectangle {
        visible: root.panelConfigOpen && root.missionVisible
        x: 10
        y: 108
        width: parent.width - 20
        height: 104
        color: root.selectedSurface
        border.color: root.cyan
        border.width: 1
        z: 10

        Text {
            x: 12
            y: 10
            text: "ИНСТРУМЕНТЫ ПАНЕЛИ"
            color: root.secondary
            font.family: "B612"
            font.pixelSize: 12
            font.bold: true
        }

        Column {
            x: 12
            y: 34
            width: parent.width - 24
            spacing: 7

            Repeater {
                model: [
                    { "label": "Анализ миссии", "key": "analysis", "enabled": root.analysisVisible },
                    { "label": "Шаблоны миссий", "key": "templates", "enabled": root.templatesExpanded }
                ]

                delegate: Item {
                    required property var modelData
                    width: parent.width
                    height: 22

                    Text {
                        anchors.left: parent.left
                        anchors.right: toggleTrack.left
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.label
                        color: modelData.enabled ? root.text : root.secondary
                        font.family: "B612"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        id: toggleTrack
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        width: 34
                        height: 18
                        radius: 9
                        color: modelData.enabled ? "#64FF00" : "#263747"
                        border.color: modelData.enabled ? "#64FF00" : "#536575"
                        border.width: 1

                        Rectangle {
                            x: modelData.enabled ? parent.width - width - 2 : 2
                            anchors.verticalCenter: parent.verticalCenter
                            width: 14
                            height: 14
                            radius: 7
                            color: modelData.enabled ? "#082014" : "#BFBFBF"
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (modelData.key === "analysis") {
                                root.analysisVisible = !root.analysisVisible
                            } else if (modelData.key === "templates") {
                                // Toggle visibility of the selected mission templates.
                                root.templatesExpanded = !root.templatesExpanded
                            }
                        }
                    }
                }
            }
        }
    }

    Flickable {
        id: templateList
        visible: (root.missionVisible || root.missionCreationMode) && root.templatesExpanded && !root.panelConfigOpen
        x: 10
        y: 92
        width: parent.width - 20
        height: Math.max(0, parent.height - y - createMissionButton.height - 20)
        contentWidth: width
        contentHeight: templateColumn.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: templateColumn
            width: templateList.width
            spacing: 3
            Repeater {
                model: ["Картографирование территории","3D-картография и реконструкция","Инспекция объектов и инфраструктуры","Мониторинг строительства","Мониторинг территории и периметра","Поиск и спасение","Пожарный мониторинг и ЧС","Экологический и природный мониторинг","Сельское хозяйство","Доставка грузов","Ретрансляция связи","Аэрофотосъёмка и медиапроизводство","Обнаружение и наблюдение за БПЛА (C-UAS)"]
                delegate: Rectangle {
                    required property int index
                    required property string modelData
                    width: templateColumn.width
                    height: 56
                    radius: 3
                    color: index === root.selectedTemplate ? root.selectedSurface : root.card
                    border.color: index === root.selectedTemplate ? root.cyan : root.divider
                    border.width: 1
                    Rectangle {
                        visible: index === root.selectedTemplate
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: 3
                        color: root.cyan
                    }
                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 30
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: modelData
                        color: root.text
                        font.family: "B612"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    Text {
                        visible: index === root.selectedTemplate
                        x: parent.width - 30
                        anchors.verticalCenter: parent.verticalCenter
                        text: "✓"
                        color: root.cyan
                        font.family: "B612 Mono"
                        font.pixelSize: 15
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.selectedTemplate = index
                    }
                }
            }
        }
    }

    Rectangle {
        id: createMissionButton
        x: 10
        y: parent.height - height - 12
        width: parent.width - 20
        height: 44
        radius: 2
        color: root.card
        border.color: root.divider
        border.width: 1
        z: 30

        Text {
            anchors.fill: parent
            text: "СОЗДАТЬ МИССИЮ"
            color: "#64FF00"
            font.family: "B612"
            font.pixelSize: 12
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.missionState = "MANUAL"
        }
    }

}
