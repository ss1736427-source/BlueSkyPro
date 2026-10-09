import QtQuick

Item {
    id: root

    // BlueSky PRO — schematic Flight Chart workspace.
    // Visual preview only: not a geographic map or flight-authoritative data.
    property color bg: "#050A12"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color muted: "#7F7F7F"
    property color divider: "#1B2A3A"
    property color cyan: "#32FFFF"
    property bool missionVisible: true
    property bool manualCreationMode: false
    // The map/mission editor sets this only when the composed mission is complete.
    property bool manualCompositionComplete: false
    // Route points are supplied by the mission profile, not hard-coded in the map view.
    property var routeCoordinates: []
    // Pan offset for mouse-driven map dragging (visual preview interaction).
    // View state can be owned by MainContent so it survives template/tool changes.
    property bool useExternalMapState: false
    property real mapPanX: 0
    property real mapPanY: 0
    property real mapZoom: 1.0
    signal mapViewChangeRequested(real panX, real panY, real zoom)

    function setMapView(panX, panY, zoom) {
        if (useExternalMapState) {
            mapViewChangeRequested(panX, panY, zoom)
        } else {
            mapPanX = panX
            mapPanY = panY
            mapZoom = zoom
        }
        chartCanvas.requestPaint()
    }

    property real dragStartMouseX: 0
    property real dragStartMouseY: 0
    property real dragStartPanX: 0
    property real dragStartPanY: 0
    property real pinchStartZoom: 1.0
    property real pinchStartPanX: 0
    property real pinchStartPanY: 0
    property real pinchStartX: 0
    property real pinchStartY: 0

    function zoomAt(factor, x, y) {
        var nextZoom = Math.max(0.5, Math.min(4.0, mapZoom * factor))
        var appliedFactor = nextZoom / mapZoom
        var nextPanX = x - (x - mapPanX) * appliedFactor
        var nextPanY = y - (y - mapPanY) * appliedFactor
        setMapView(nextPanX, nextPanY, nextZoom)
    }

    signal manualCompositionCompleted()
    signal mapDoubleClicked()

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    // Google Maps Platform Map Tiles API is the authoritative basemap.
    GoogleMapView {
        id: googleMap
        anchors.fill: parent
        z: 1
        routeCoordinates: root.routeCoordinates
    }

    Canvas {
        id: chartCanvas
        anchors.fill: parent
        visible: false
        renderTarget: Canvas.Image
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            var ctx = getContext("2d")
            var w = width
            var h = height
            ctx.clearRect(0, 0, w, h)
            ctx.fillStyle = root.bg
            ctx.fillRect(0, 0, w, h)

            // Pan map content while keeping the surrounding UI overlays fixed.
            ctx.save()
            ctx.translate(root.mapPanX, root.mapPanY)
            ctx.scale(root.mapZoom, root.mapZoom)

            var step = 66
            ctx.lineWidth = 1
            for (var x = 0; x <= w; x += step) {
                ctx.beginPath()
                ctx.strokeStyle = (Math.round(x / step) % 5 === 0) ? "#1B2A3A" : "#111E2C"
                ctx.moveTo(x, 0)
                ctx.lineTo(x, h)
                ctx.stroke()
            }
            for (var y = 0; y <= h; y += step) {
                ctx.beginPath()
                ctx.strokeStyle = (Math.round(y / step) % 5 === 0) ? "#1B2A3A" : "#111E2C"
                ctx.moveTo(0, y)
                ctx.lineTo(w, y)
                ctx.stroke()
            }

            // Restricted area (illustrative only).
            var rx = w * 0.57
            var ry = h * 0.22
            var rw = Math.min(194, w * 0.23)
            var rh = Math.min(154, h * 0.24)
            ctx.fillStyle = "#30121B"
            ctx.strokeStyle = "#8B2637"
            ctx.lineWidth = 1
            ctx.fillRect(rx, ry, rw, rh)
            ctx.strokeRect(rx, ry, rw, rh)
            ctx.strokeStyle = "#5A202D"
            for (var sy = ry + 14; sy < ry + rh; sy += 24) {
                ctx.beginPath()
                ctx.moveTo(rx + 8, sy)
                ctx.lineTo(rx + rw - 8, sy)
                ctx.stroke()
            }

            // Schematic route. Center the complete route geometry in the
            // currently available map viewport, so opening/closing side panels
            // does not leave the mission area visually off-center.
            var pts = [
                {x: w * 0.16, y: h * 0.73},
                {x: w * 0.30, y: h * 0.54},
                {x: w * 0.42, y: h * 0.62},
                {x: w * 0.52, y: h * 0.42},
                {x: w * 0.73, y: h * 0.69}
            ]
            var minX = pts[0].x
            var maxX = pts[0].x
            var minY = pts[0].y
            var maxY = pts[0].y
            for (var b = 1; b < pts.length; ++b) {
                minX = Math.min(minX, pts[b].x)
                maxX = Math.max(maxX, pts[b].x)
                minY = Math.min(minY, pts[b].y)
                maxY = Math.max(maxY, pts[b].y)
            }
            var offsetX = w / 2 - (minX + maxX) / 2
            var offsetY = h / 2 - (minY + maxY) / 2
            for (var p = 0; p < pts.length; ++p) {
                pts[p].x += offsetX
                pts[p].y += offsetY
            }

            ctx.beginPath()
            ctx.moveTo(pts[0].x, pts[0].y)
            for (var i = 1; i < pts.length; ++i)
                ctx.lineTo(pts[i].x, pts[i].y)
            ctx.strokeStyle = root.cyan
            ctx.lineWidth = 2
            ctx.lineJoin = "round"
            ctx.stroke()

            for (var j = 0; j < pts.length; ++j) {
                ctx.beginPath()
                ctx.arc(pts[j].x, pts[j].y, j === 0 || j === pts.length - 1 ? 4.5 : 3.5, 0, Math.PI * 2)
                ctx.fillStyle = j === 0 ? "#64FF00" : (j === pts.length - 1 ? "#FFD43B" : root.cyan)
                ctx.fill()
            }

            ctx.restore()
        }
    }

    Text {
        visible: true
        x: 18
        y: 16
        text: "FLIGHT CHART"
        color: root.secondary
        font.family: "B612"
        font.pixelSize: 14
        font.bold: true
    }

    Rectangle {
        visible: true
        x: 18
        y: 44
        width: 108
        height: 24
        color: "#0C1725"
        border.color: "#24384B"
        border.width: 1
        Text {
            anchors.centerIn: parent
            text: "SCHEMATIC VIEW"
            color: "#FFD43B"
            font.family: "B612 Mono"
            font.pixelSize: 9
        }
    }

    Rectangle {
        visible: true
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 18
        anchors.rightMargin: 18
        width: 128
        height: 54
        color: "#08111D"
        border.color: "#24384B"
        border.width: 1
        Column {
            anchors.centerIn: parent
            spacing: 3
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "WIND"
                color: root.muted
                font.family: "B612 Mono"
                font.pixelSize: 9
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "↗  — m/s"
                color: root.cyan
                font.family: "B612 Mono"
                font.pixelSize: 12
            }
        }
    }

    Rectangle {
        visible: true
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 18
        anchors.bottomMargin: 14
        width: 286
        height: 26
        color: "#050A12"
        opacity: 0.94
        Row {
            anchors.fill: parent
            spacing: 14
            anchors.leftMargin: 2
            Repeater {
                model: [
                    { label: "START", color: "#64FF00" },
                    { label: "WAYPOINT", color: root.cyan },
                    { label: "FINISH", color: "#FFD43B" },
                    { label: "RESTRICTED", color: "#FF4D5A" }
                ]
                delegate: Row {
                    spacing: 5
                    anchors.verticalCenter: parent.verticalCenter
                    Rectangle {
                        width: 7
                        height: 7
                        radius: 4
                        color: modelData.color
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: modelData.label
                        color: root.secondary
                        font.family: "B612 Mono"
                        font.pixelSize: 8
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }

    Text {
        visible: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 14
        text: "SCHEMATIC ROUTE • NOT FOR FLIGHT EXECUTION"
        color: root.muted
        font.family: "B612 Mono"
        font.pixelSize: 9
    }

    MouseArea {
        visible: false
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

        onPressed: function(mouse) {
            root.dragStartMouseX = mouse.x
            root.dragStartMouseY = mouse.y
            root.dragStartPanX = root.mapPanX
            root.dragStartPanY = root.mapPanY
        }

        onPositionChanged: function(mouse) {
            if (pressed) {
                root.setMapView(
                    root.dragStartPanX + mouse.x - root.dragStartMouseX,
                    root.dragStartPanY + mouse.y - root.dragStartMouseY,
                    root.mapZoom
                )
            }
        }

        onDoubleClicked: root.mapDoubleClicked()
    }

    Text {
        visible: !root.missionVisible && !root.manualCreationMode
        anchors.centerIn: parent
        text: "НЕТ АКТИВНОЙ МИССИИ"
        color: root.muted
        font.family: "B612 Mono"
        font.pixelSize: 12
    }

    Text {
        visible: root.manualCreationMode
        anchors.horizontalCenter: parent.horizontalCenter
        y: 24
        text: "РУЧНОЕ СОЗДАНИЕ МИССИИ · M"
        color: "#64FF00"
        font.family: "B612 Mono"
        font.pixelSize: 12
        font.bold: true
    }

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
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 44
        width: 260
        height: 254
        title: "MAP SETTINGS"
        tools: ["Base Map", "Airspace / Restrictions", "NOTAM", "Weather Layers", "Route / Waypoints", "UAV Display", "Planned / Actual Track", "Map Interaction"]
        onClosed: open = false
    }
}
