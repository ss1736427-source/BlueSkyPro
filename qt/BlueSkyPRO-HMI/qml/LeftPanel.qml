import QtQuick

Item {
    id: root

    TextMetrics {
        id: widestTemplateText
        font.family: "Noto Sans"
        font.pixelSize: 12
        font.bold: true
        text: "Картографирование территории"
    }

    TextMetrics {
        id: headerTitleText
        font.family: "Noto Sans"
        font.pixelSize: 13
        font.bold: true
        text: "Миссии"
    }

    // Width is driven by the widest visible content and the fixed right-side header controls.
    implicitWidth: Math.max(270, Math.min(420,
        Math.max(widestTemplateText.width + 44, headerTitleText.width + 84)))
    implicitHeight: 520
    // When hosted by MainContent the panel height is supplied by top/bottom anchors,
    // so it follows the workspace height automatically.

    property color bg: "#08111D"
    property color card: "#0C1725"
    property color selectedSurface: "#111F30"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color muted: "#7F7F7F"
    property color cyan: "#32FFFF"
    property color divider: "#7F7F7F"

    property bool showTopBorder: true
    property bool showRightBorder: true
    property bool showBottomBorder: true
    property bool showLeftBorder: true

    property bool missionVisible: true
    property bool missionCreationMode: false
    readonly property string newMissionType: "M"
    property string missionId: "BS-260920-A-001"
    property string missionSummary: "3D картография территории"
    property bool missionIdExpanded: false
    readonly property string missionShortId: {
        var parts = missionId.split("-")
        return parts.length >= 4 ? parts[2] + "-" + parts[3] : missionId
    }
    property bool templatesExpanded: true
    property bool panelConfigOpen: false
    property int selectedTemplate: 2
    // Indices assigned to the current automatic mission; manual mode starts with no template selected.
    property var missionTemplateIndices: [2]
    property var manualTemplateSelection: []

    onMissionCreationModeChanged: {
        if (missionCreationMode)
            manualTemplateSelection = []
    }

    function templateIsVisible(index) {
        return missionCreationMode || missionTemplateIndices.indexOf(index) >= 0
    }

    function templateIsSelected(index) {
        return missionCreationMode
            ? manualTemplateSelection.indexOf(index) >= 0
            : missionTemplateIndices.indexOf(index) >= 0
    }

    function toggleManualTemplate(index) {
        var next = manualTemplateSelection.slice()
        var position = next.indexOf(index)
        if (position >= 0)
            next.splice(position, 1)
        else
            next.push(index)
        manualTemplateSelection = next
        selectedTemplate = index
        templateSelected(index)
        manualTemplateSelectionCommitted(manualTemplateSelection)
    }
    property bool analysisVisible: true
    property bool instrumentsVisible: false
    property bool atcVisible: false
    property bool diagnosticsVisible: false

    signal hideMissionRequested()
    signal restoreMissionRequested()
    signal missionTemplateMenuRequested()
    signal templateSelected(int index)
    signal manualTemplateSelectionCommitted(var indices)
    signal createMissionRequested()
    signal panelConfigurationChanged()
    signal missionHidden()

    ListModel {
        id: templateModel
        ListElement { title: "Картографирование территории"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Инспекция объектов (здания, ЛЭП, трубопроводы)"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "3D-картография / реконструкция"; subtitle: ""; accent: "#32FFFF" }
        ListElement { title: "Мониторинг территории"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Поиск человека"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Пожарный мониторинг"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Доставка"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Drone-in-a-Box"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "BVLOS-мониторинг"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Экологический мониторинг"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Ретрансляция связи"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "C-UAS — обнаружение БПЛА"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Автоматическое обнаружение объектов"; subtitle: ""; accent: "#BFBFBF" }
        ListElement { title: "Групповая / роёвая миссия"; subtitle: ""; accent: "#BFBFBF" }
    }
    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    // Edge ownership is configurable. When a neighboring component already
    // draws a shared separator, disable this edge to prevent a doubled stroke.
    Rectangle {
        visible: root.showTopBorder
        x: 0
        y: 0
        width: parent.width
        height: 1
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        visible: root.showRightBorder
        x: parent.width - 1
        y: 0
        width: 1
        height: parent.height
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        visible: root.showBottomBorder
        x: 0
        y: parent.height - 1
        width: parent.width
        height: 1
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        visible: root.showLeftBorder
        x: 0
        y: 0
        width: 1
        height: parent.height
        color: root.cyan
        antialiasing: false
    }

    // Panel title bar: distinct hierarchy within the panel, using the
    // controlled selected surface and one cyan outline.
    Rectangle {
        x: 10
        y: 8
        width: parent.width - 20
        height: 38
        color: root.selectedSurface
        border.color: root.cyan
        border.width: 1

        Text {
            x: 12
            anchors.verticalCenter: parent.verticalCenter
            rightPadding: 70
            text: "Миссии"
            color: root.text
            font.family: "B612"
            font.pixelSize: 14
            font.bold: true
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            Text {
                width: 18
                horizontalAlignment: Text.AlignHCenter
                text: "+"
                color: root.missionVisible ? root.muted : root.cyan
                font.family: "B612 Mono"
                font.pixelSize: 18
                font.bold: true

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (!root.missionVisible)
                            root.restoreMissionRequested()
                    }
                }
            }

            Text {
                width: 18
                horizontalAlignment: Text.AlignHCenter
                text: "≡"
                color: root.panelConfigOpen ? root.cyan : root.secondary
                font.family: "B612 Mono"
                font.pixelSize: 16

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        root.panelConfigOpen = !root.panelConfigOpen
                        root.missionTemplateMenuRequested()
                    }
                }
            }
        }
    }

    // Compact mission row: ID, summary and hide action are separated inside one contour.
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
            width: root.missionIdExpanded ? 142 : 48
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            text: root.missionIdExpanded ? root.missionId : root.missionShortId
            color: root.text
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
            width: Math.max(0, parent.width - x - 98)
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            text: root.missionSummary
            color: root.secondary
            font.family: "Noto Sans"
            font.pixelSize: 10
        }

        Rectangle {
            x: parent.width - 91
            y: 8
            width: 1
            height: parent.height - 16
            color: root.divider
        }

        Rectangle {
            id: hideMissionButton
            x: parent.width - 82
            width: 72
            height: 26
            anchors.verticalCenter: parent.verticalCenter
            radius: 2
            color: root.selectedSurface
            border.color: root.divider
            border.width: 1

            Text {
                anchors.fill: parent
                text: "⊘  СКРЫТЬ"
                color: root.secondary
                font.family: "B612 Mono"
                font.pixelSize: 9
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.hideMissionRequested()
            }
        }
    }

    // Panel configuration is presentation-only and independent of Bottom Toolbar configuration.
    Rectangle {
        visible: root.panelConfigOpen && root.missionVisible
        x: 10
        y: 90
        width: parent.width - 20
        height: 154
        color: root.selectedSurface
        border.color: root.cyan
        border.width: 1
        z: 20

        Text {
            x: 12
            y: 10
            text: "ИНСТРУМЕНТЫ ПАНЕЛИ"
            color: root.secondary
            font.family: "Noto Sans"
            font.pixelSize: 10
            font.bold: true
        }

        Column {
            x: 12
            y: 32
            width: parent.width - 24
            spacing: 7

            Repeater {
                model: [
                    { "label": "Анализ миссии", "key": "analysis", "enabled": root.analysisVisible },
                    { "label": "Приборы", "key": "instruments", "enabled": root.instrumentsVisible },
                    { "label": "УВД / Связь", "key": "atc", "enabled": root.atcVisible },
                    { "label": "Расширенная диагностика", "key": "diagnostics", "enabled": root.diagnosticsVisible }
                ]

                delegate: Text {
                    required property var modelData
                    width: parent.width
                    text: (modelData.enabled ? "✓ " : "○ ") + modelData.label
                    color: modelData.enabled ? root.cyan : root.secondary
                    font.family: "Noto Sans"
                    font.pixelSize: 10

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (modelData.key === "analysis") root.analysisVisible = !root.analysisVisible
                            else if (modelData.key === "instruments") root.instrumentsVisible = !root.instrumentsVisible
                            else if (modelData.key === "atc") root.atcVisible = !root.atcVisible
                            else if (modelData.key === "diagnostics") root.diagnosticsVisible = !root.diagnosticsVisible
                            root.panelConfigurationChanged()
                        }
                    }
                }
            }
        }
    }

    // Canonical task templates. Detailed geometry modes (corridor, linear route,
    // point of interest) are selected inside the mission setup, not as top-level tasks.
    Flickable {
        id: templateList
        visible: (root.missionVisible || root.missionCreationMode) && root.templatesExpanded && !root.panelConfigOpen
        x: 10
        y: root.missionVisible ? 110 : 50
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
                model: templateModel
                delegate: Rectangle {
                    required property int index
                    required property string title
                    required property string subtitle
                    required property string accent
                    width: templateColumn.width
                    height: 56
                    radius: 3
                    color: root.templateIsSelected(index) ? root.selectedSurface : root.card
                    border.color: root.templateIsSelected(index) ? root.cyan : root.divider
                    border.width: 1

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: 3
                        radius: 2
                        color: root.templateIsSelected(index) ? root.cyan : "transparent"
                    }
                    Text {
                        x: 16
                        width: parent.width - 52
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                        text: title
                        color: root.text
                        font.family: "Noto Sans"
                        font.pixelSize: 12
                        font.bold: true
                    }
                    Text {
                        visible: root.templateIsSelected(index)
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
                        onClicked: {
                            if (root.missionCreationMode)
                                root.toggleManualTemplate(index)
                            else {
                                root.selectedTemplate = index
                                root.templateSelected(index)
                            }
                        }
                    }
                }
            }
        }
    }

    // Expanded mission context remains available when templates are collapsed.
    Column {
        visible: root.missionVisible && !root.templatesExpanded && !root.panelConfigOpen
        x: 16
        y: 98
        width: parent.width - 32
        spacing: 10

        Text {
            text: root.missionIdExpanded ? root.missionId : root.missionShortId
            color: root.text
            font.family: "B612 Mono"
            font.pixelSize: 16
            font.bold: true
        }

        Text {
            text: "Обследование территории"
            color: root.text
            font.family: "Noto Sans"
            font.pixelSize: 13
        }

        Text {
            text: "4 БПЛА  |  BVLOS  |  учёт ветра"
            color: root.secondary
            font.family: "Noto Sans"
            font.pixelSize: 11
        }
    }

    Text {
        visible: root.missionVisible && !root.templatesExpanded && !root.panelConfigOpen
        x: 16
        y: 162
        text: "ID МИССИИ  |  неизменяемый"
        color: root.secondary
        font.family: "Noto Sans"
        font.pixelSize: 11
        font.bold: true
    }

    Text {
        visible: root.missionVisible && !root.templatesExpanded && !root.panelConfigOpen
        x: 16
        y: 184
        text: root.missionId
        color: root.text
        font.family: "B612 Mono"
        font.pixelSize: 11
    }

    Text {
        visible: root.missionVisible && !root.templatesExpanded
        x: 16
        y: 226
        text: "ИНСТРУМЕНТЫ ПАНЕЛИ"
        color: root.secondary
        font.family: "Noto Sans"
        font.pixelSize: 11
        font.bold: true
    }

    Text {
        visible: root.missionVisible && !root.templatesExpanded
        x: 16
        y: 250
        text: "Анализ  |  Приборы  |  УВД  |  Диагностика"
        color: root.secondary
        font.family: "Noto Sans"
        font.pixelSize: 10
    }

    Text {
        visible: root.missionVisible && !root.templatesExpanded
        x: 16
        y: 270
        text: "Weather  |  Obstacles  |  Airspace"
        color: root.secondary
        font.family: "Noto Sans"
        font.pixelSize: 10
    }

    // Fixed primary action: remains at the bottom in every mission-panel state.
    Rectangle {
        id: createMissionButton
        x: 10
        y: parent.height - height - 12
        width: parent.width - 20
        height: 44
        radius: 2
        color: "#64FF00"
        border.color: "#64FF00"
        border.width: 1
        z: 30

        Text {
            anchors.fill: parent
            text: "СОЗДАТЬ МИССИЮ"
            color: "#050A12"
            font.family: "B612"
            font.pixelSize: 12
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.panelConfigOpen = false
                root.missionIdExpanded = false
                root.createMissionRequested()
            }
        }
    }

}
