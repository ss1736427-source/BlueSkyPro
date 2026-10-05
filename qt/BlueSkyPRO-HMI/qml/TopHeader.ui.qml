import QtQuick

Item {
    id: root

    // Top Header — composition workbench variant for Qt Design Studio.
    // Design Studio workbench size only; runtime width remains adaptive through parent anchors.
    implicitWidth: 1920
    implicitHeight: root.headerHeight

    property int headerHeight: 86
    property color bg: "#050A12"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color muted: "#7F7F7F"
    property color green: "#64FF00"
    property color amber: "#FFD339"
    property color red: "#FF1E14"
    property color divider: "#111F30"
    property color panel: "#08111D"
    property color accent: "#32FFFF"
    property int borderWidth: 1
    property int borderRadius: 4

    property string etd: "10:30"
    property string tot: "—"
    property string trip: "—"
    property string eta: "11:48"
    property bool ready: true
    property bool warningActive: true
    property int warningCount: 0
    property bool warningBlinkOn: true

    Timer {
        id: warningBlinkTimer
        interval: 550
        repeat: true
        running: root.warningCount > 0
        onTriggered: root.warningBlinkOn = !root.warningBlinkOn
    }

    onWarningCountChanged: {
        if (warningCount <= 0)
            warningBlinkOn = true
    }
    // Supplied by the authenticated pilot profile; replace placeholder with actual profile binding.
    property string operatorLabel: "Фамилия И.О."

    // Adjustable composition parameters — working values, not frozen tokens.
    // Header anchors align exactly with the side panels below.
    property int anchorWidth: 164
    property int leftAnchorWidth: anchorWidth
    property int rightAnchorWidth: anchorWidth
    property int centralSectorWidth: 112
    property int sectorGap: 0
    property int anchorPadding: 2
    property int logoScale: 100
    property int logoVisualWidth: 266
    property int logoVisualHeight: 50
    property int operatorIconSize: 34
    property int operatorLabelSize: 11
    property int headingSize: 11
    property int valueSize: 18
    property int headingValueGap: 3
    // One canonical stroke token for every Header line.
    property int headerStrokeWidth: 1
    // Design Studio preview is commonly rendered below 100% scene scale.
    // Keep divider geometry on integer logical coordinates and disable edge AA.
    property int structuralDividerHeight: 54
    // Canonical one-pixel stroke; shared panel outlines use the same logical width.
    property real devicePixelRatio: Screen.devicePixelRatio
    property real pixelStrokeWidth: 1

    property var metricOrder: ["ETD", "TOT", "TRIP", "ETA", "READY", "WARNING"]
    readonly property int visibleMetricCount: metricOrder.filter(function(name) {
        return panelSettingsPopup.enabledTools.indexOf(name) >= 0
    }).length

    function metricX(name) {
        var visibleBefore = 0
        for (var i = 0; i < metricOrder.length; ++i) {
            if (metricOrder[i] === name)
                break
            if (panelSettingsPopup.enabledTools.indexOf(metricOrder[i]) >= 0)
                ++visibleBefore
        }
        return centralComposition.width * visibleBefore / Math.max(1, visibleMetricCount)
    }

    property bool tabletVariant: width < 1500

    Rectangle {
        id: headerSurface
        anchors.fill: parent
        color: root.panel
        border.width: 0
    }

    // LEFT ANCHOR — width is a composition parameter and contains the scalable logo.
    Item {
        id: logoBlock
        width: Math.max(0, Math.min(root.leftAnchorWidth, parent.width))
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Item {
            width: Math.min(Math.round(root.logoVisualWidth * root.logoScale / 100),
                            logoBlock.width - 2 * root.anchorPadding)
            height: Math.min(Math.round(root.logoVisualHeight * root.logoScale / 100),
                             logoBlock.height - 2 * root.anchorPadding)
            anchors.centerIn: parent

            Image {
                anchors.fill: parent
                source: Qt.resolvedUrl("assets/bluesky_pro_logo.svg")
                fillMode: Image.PreserveAspectFit
                // Render from the full-resolution master and use mipmaps for clean downsampling.
                smooth: true
                mipmap: true
                cache: false
                sourceSize.width: 2048
                sourceSize.height: 768
            }
        }
    }

    // RIGHT ANCHOR — outer geometry is intentionally identical to LOGO anchor.
    Item {
        id: operatorBlock
        // Operator is a permanent right anchor, not a hideable header metric.
        visible: true
        width: Math.max(0, Math.min(root.rightAnchorWidth, parent.width))
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Item {
            anchors.centerIn: parent
            width: parent.width - 2 * root.anchorPadding
            height: parent.height - 2 * root.anchorPadding

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: root.divider
                border.width: root.headerStrokeWidth
                radius: 3
            }

            Column {
                anchors.centerIn: parent
                spacing: 0

                Text {
                    text: root.operatorLabel
                    color: root.secondary
                    font.family: "B612"
                    font.pixelSize: Math.min(14, Math.max(10, root.operatorLabelSize + 2))
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // CENTRAL COMPOSITION — fills the adaptive space between equal-width anchors.
    // Six equal sectors preserve the geometric center between TRIP and ETA.
    Item {
        id: centralComposition
        anchors.left: logoBlock.right
        anchors.right: operatorBlock.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        HeaderSector {
            id: etdSector
            visible: panelSettingsPopup.enabledTools.indexOf("ETD") >= 0
            x: root.metricX("ETD")
            width: centralComposition.width / Math.max(1, root.visibleMetricCount)
            height: parent.height
            title: "ETD"
            value: root.etd
            valueColor: root.secondary
            headingSize: root.headingSize
            valueSize: root.valueSize
            headingValueGap: root.headingValueGap
        }

        HeaderSector {
            id: totSector
            visible: panelSettingsPopup.enabledTools.indexOf("TOT") >= 0
            x: root.metricX("TOT")
            width: centralComposition.width / Math.max(1, root.visibleMetricCount)
            height: parent.height
            title: "TOT"
            value: root.tot
            valueColor: root.secondary
            headingSize: root.headingSize
            valueSize: root.valueSize
            headingValueGap: root.headingValueGap
        }

        HeaderSector {
            id: tripSector
            visible: panelSettingsPopup.enabledTools.indexOf("TRIP") >= 0
            x: root.metricX("TRIP")
            width: centralComposition.width / Math.max(1, root.visibleMetricCount)
            height: parent.height
            title: "TRIP"
            value: root.trip
            valueColor: root.secondary
            headingSize: root.headingSize
            valueSize: root.valueSize
            headingValueGap: root.headingValueGap
        }

        HeaderSector {
            id: etaSector
            visible: panelSettingsPopup.enabledTools.indexOf("ETA") >= 0
            x: root.metricX("ETA")
            width: centralComposition.width / Math.max(1, root.visibleMetricCount)
            height: parent.height
            title: "ETA"
            value: root.eta
            valueColor: root.secondary
            headingSize: root.headingSize
            valueSize: root.valueSize
            headingValueGap: root.headingValueGap
        }

        HeaderSector {
            id: readySector
            visible: panelSettingsPopup.enabledTools.indexOf("READY") >= 0
            x: root.metricX("READY")
            width: centralComposition.width / Math.max(1, root.visibleMetricCount)
            height: parent.height
            title: "READY"
            value: root.ready ? "READY" : "NOT READY"
            valueColor: root.ready ? root.green : root.amber
            headingSize: root.headingSize
            valueSize: root.valueSize
            headingValueGap: root.headingValueGap
        }

        HeaderSector {
            id: warningSector
            visible: panelSettingsPopup.enabledTools.indexOf("WARNING") >= 0
            x: root.metricX("WARNING")
            width: centralComposition.width / Math.max(1, root.visibleMetricCount)
            height: parent.height
            title: "WARNING"
            value: root.warningCount > 0 ? String(root.warningCount) : ""
            valueColor: root.warningCount > 0 ? root.amber : root.muted
            opacity: root.warningCount > 0 && !root.warningBlinkOn ? 0.25 : 1.0
            headingSize: root.headingSize
            valueSize: root.valueSize
            headingValueGap: root.headingValueGap
        }
    }

    // WARNING is an actionable header control: open/close the map alert review.
    signal warningClicked()

    MouseArea {
        id: warningClickArea
        visible: warningSector.visible
        x: centralComposition.x + root.metricX("WARNING")
        y: 0
        width: centralComposition.width / Math.max(1, root.visibleMetricCount)
        height: parent.height
        z: 80
        cursorShape: root.warningCount > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.warningClicked()
    }

    // Adaptive dividers follow only the currently enabled metrics.
    Repeater {
        model: Math.max(0, root.visibleMetricCount - 1)
        delegate: Rectangle {
            required property int index
            x: Math.round((centralComposition.x
                + centralComposition.width * (index + 1) / Math.max(1, root.visibleMetricCount))
                * root.devicePixelRatio) / root.devicePixelRatio
            y: Math.round(((root.height - Math.min(root.structuralDividerHeight, root.height - 16)) / 2)
                * root.devicePixelRatio) / root.devicePixelRatio
            width: root.pixelStrokeWidth
            height: Math.round(Math.min(root.structuralDividerHeight, root.height - 16)
                * root.devicePixelRatio) / root.devicePixelRatio
            color: root.accent
            antialiasing: false
            z: 90
        }
    }

    // Fixed anchor boundaries: never move or disappear when metrics are toggled.
    Rectangle {
        x: Math.round(logoBlock.width * root.devicePixelRatio) / root.devicePixelRatio
        y: Math.round(((root.height - Math.min(root.structuralDividerHeight, root.height - 16)) / 2)
            * root.devicePixelRatio) / root.devicePixelRatio
        width: root.pixelStrokeWidth
        height: Math.round(Math.min(root.structuralDividerHeight, root.height - 16)
            * root.devicePixelRatio) / root.devicePixelRatio
        color: root.accent
        antialiasing: false
        z: 95
    }

    Rectangle {
        x: Math.round((centralComposition.x + centralComposition.width)
            * root.devicePixelRatio) / root.devicePixelRatio
        y: Math.round(((root.height - Math.min(root.structuralDividerHeight, root.height - 16)) / 2)
            * root.devicePixelRatio) / root.devicePixelRatio
        width: root.pixelStrokeWidth
        height: Math.round(Math.min(root.structuralDividerHeight, root.height - 16)
            * root.devicePixelRatio) / root.devicePixelRatio
        color: root.accent
        antialiasing: false
        z: 95
    }

    // Outer frame: explicit 1px primitives, same as every structural divider.
    // Do not use Rectangle.border here: its rasterization differs from the divider
    // rectangles, especially at Design Studio zoom levels.
    Rectangle {
        x: 0
        y: 0
        width: parent.width
        height: root.headerStrokeWidth
        antialiasing: false
        color: root.accent
        z: 100
    }

    Rectangle {
        x: 0
        y: parent.height - root.headerStrokeWidth
        width: parent.width
        height: root.headerStrokeWidth
        antialiasing: false
        color: root.accent
        z: 100
    }

    Rectangle {
        x: 0
        y: 0
        width: root.headerStrokeWidth
        height: parent.height
        antialiasing: false
        color: root.accent
        z: 100
    }

    Rectangle {
        x: parent.width - root.headerStrokeWidth
        y: 0
        width: root.headerStrokeWidth
        height: parent.height
        antialiasing: false
        color: root.accent
        z: 100
    }

    // Central composition always occupies the exact adaptive space between the two equal anchors.


    PanelSettingsButton {
        id: panelSettings
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        z: 400
        onClicked: panelSettingsPopup.open = !panelSettingsPopup.open
    }

    PanelSettingsPopup {
        id: panelSettingsPopup
        anchors.top: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.topMargin: 8
        width: Math.min(
                   Math.max(220, panelSettingsPopup.contentWidth),
                   Math.max(180, Math.min(420, Screen.width - 16, root.width - 16)))
        height: Math.min(
                    52 + panelSettingsPopup.tools.length * 30 + 48,
                    Math.max(180, Screen.height - 24))
        title: "HEADER SETTINGS"
        tools: ["ETD", "TOT", "TRIP", "ETA", "READY", "WARNING"]
        onClosed: open = false
    }
}
