import QtQuick
import QtQml

Item {
    id: root

    property real centerLatitude: 55.7558
    property real centerLongitude: 37.6176
    property int zoomLevel: 10
    property string sessionToken: ""
    property string attribution: "Google Maps"
    property string mapStatus: "INITIALIZING"
    property var routeCoordinates: [
        { lat: 55.7600, lon: 37.6000 },
        { lat: 55.7350, lon: 37.6250 },
        { lat: 55.7550, lon: 37.6550 },
        { lat: 55.7800, lon: 37.6400 },
        { lat: 55.7700, lon: 37.5900 }
    ]

    readonly property int tileSize: 256
    property real panOffsetX: 0
    property real panOffsetY: 0
    property var tiles: []

    signal viewChanged(real latitude, real longitude, int zoom)

    function clamp(value, minimum, maximum) {
        return Math.max(minimum, Math.min(maximum, value))
    }

    function worldSize() {
        return tileSize * Math.pow(2, zoomLevel)
    }

    function longitudeToWorld(lon) {
        return (lon + 180.0) / 360.0 * worldSize()
    }

    function latitudeToWorld(lat) {
        var latitude = clamp(lat, -85.05112878, 85.05112878)
        var sinLat = Math.sin(latitude * Math.PI / 180.0)
        return (0.5 - Math.log((1 + sinLat) / (1 - sinLat)) / (4 * Math.PI)) * worldSize()
    }

    function worldToLongitude(x) {
        var size = worldSize()
        var wrapped = ((x % size) + size) % size
        return wrapped / size * 360.0 - 180.0
    }

    function worldToLatitude(y) {
        var size = worldSize()
        var n = Math.PI - 2 * Math.PI * y / size
        return 180.0 / Math.PI * Math.atan(Math.sinh(n))
    }

    function rebuildTiles() {
        if (width <= 0 || height <= 0 || sessionToken.length === 0)
            return

        var size = worldSize()
        var cx = longitudeToWorld(centerLongitude)
        var cy = latitudeToWorld(centerLatitude)
        var firstX = Math.floor((cx - width / 2 - tileSize) / tileSize)
        var lastX = Math.floor((cx + width / 2 + tileSize) / tileSize)
        var firstY = Math.floor((cy - height / 2 - tileSize) / tileSize)
        var lastY = Math.floor((cy + height / 2 + tileSize) / tileSize)
        var result = []

        for (var ty = firstY; ty <= lastY; ++ty) {
            if (ty < 0 || ty >= Math.pow(2, zoomLevel))
                continue
            for (var tx = firstX; tx <= lastX; ++tx) {
                var wrappedX = ((tx % Math.pow(2, zoomLevel)) + Math.pow(2, zoomLevel)) % Math.pow(2, zoomLevel)
                result.push({
                    tx: tx,
                    ty: ty,
                    x: tx * tileSize - cx + width / 2 + panOffsetX,
                    y: ty * tileSize - cy + height / 2 + panOffsetY,
                    url: "https://tile.googleapis.com/v1/2dtiles/" + zoomLevel + "/" +
                         wrappedX + "/" + ty + "?session=" +
                         encodeURIComponent(sessionToken) + "&key=" +
                         encodeURIComponent(googleMapsApiKey)
                })
            }
        }
        tiles = result
    }

    function requestSession() {
        if (googleMapsApiKey.length === 0) {
            mapStatus = "API KEY REQUIRED"
            return
        }

        mapStatus = "CONNECTING"
        var request = new XMLHttpRequest()
        request.onreadystatechange = function() {
            if (request.readyState !== XMLHttpRequest.DONE)
                return

            if (request.status >= 200 && request.status < 300) {
                try {
                    var response = JSON.parse(request.responseText)
                    sessionToken = response.session || ""
                    if (sessionToken.length > 0) {
                        mapStatus = "READY"
                        rebuildTiles()
                    } else {
                        mapStatus = "SESSION ERROR"
                    }
                } catch (error) {
                    mapStatus = "SESSION ERROR"
                }
            } else {
                mapStatus = "HTTP " + request.status
            }
        }

        request.open(
            "POST",
            "https://tile.googleapis.com/v1/createSession?key=" +
            encodeURIComponent(googleMapsApiKey)
        )
        request.setRequestHeader("Content-Type", "application/json")
        request.send(JSON.stringify({
            mapType: "roadmap",
            language: "en-US",
            region: "DE"
        }))
    }

    function commitPan() {
        var size = worldSize()
        var centerX = longitudeToWorld(centerLongitude) - panOffsetX
        var centerY = latitudeToWorld(centerLatitude) - panOffsetY
        centerLongitude = worldToLongitude(centerX)
        centerLatitude = clamp(worldToLatitude(centerY), -85.0, 85.0)
        panOffsetX = 0
        panOffsetY = 0
        rebuildTiles()
        routeCanvas.requestPaint()
        viewChanged(centerLatitude, centerLongitude, zoomLevel)
    }

    onWidthChanged: rebuildTiles()
    onHeightChanged: rebuildTiles()
    onCenterLatitudeChanged: { rebuildTiles(); routeCanvas.requestPaint() }
    onCenterLongitudeChanged: { rebuildTiles(); routeCanvas.requestPaint() }
    onZoomLevelChanged: { rebuildTiles(); routeCanvas.requestPaint() }
    onSessionTokenChanged: rebuildTiles()
    onPanOffsetXChanged: routeCanvas.requestPaint()
    onPanOffsetYChanged: routeCanvas.requestPaint()

    Rectangle {
        anchors.fill: parent
        color: "#E8EEF2"
    }

    Repeater {
        model: root.tiles

        delegate: Image {
            x: modelData.x
            y: modelData.y
            width: root.tileSize
            height: root.tileSize
            source: modelData.url
            asynchronous: true
            cache: false
            fillMode: Image.Stretch
            smooth: true
        }
    }

    Canvas {
        id: routeCanvas
        anchors.fill: parent
        z: 20

        function pointForCoordinate(coordinate) {
            var centerX = root.longitudeToWorld(root.centerLongitude)
            var centerY = root.latitudeToWorld(root.centerLatitude)
            var x = root.longitudeToWorld(coordinate.lon) - centerX + width / 2 + root.panOffsetX
            var y = root.latitudeToWorld(coordinate.lat) - centerY + height / 2 + root.panOffsetY

            var size = root.worldSize()
            if (x < -size / 2)
                x += size
            else if (x > size / 2 + width)
                x -= size

            return { x: x, y: y }
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!root.routeCoordinates || root.routeCoordinates.length < 2)
                return

            ctx.beginPath()
            var first = pointForCoordinate(root.routeCoordinates[0])
            ctx.moveTo(first.x, first.y)

            for (var i = 1; i < root.routeCoordinates.length; ++i) {
                var point = pointForCoordinate(root.routeCoordinates[i])
                ctx.lineTo(point.x, point.y)
            }

            ctx.strokeStyle = "#00D9FF"
            ctx.lineWidth = 3
            ctx.lineJoin = "round"
            ctx.stroke()

            for (var j = 0; j < root.routeCoordinates.length; ++j) {
                var marker = pointForCoordinate(root.routeCoordinates[j])
                ctx.beginPath()
                ctx.arc(marker.x, marker.y, j === 0 || j === root.routeCoordinates.length - 1 ? 6 : 4, 0, Math.PI * 2)
                ctx.fillStyle = j === 0 ? "#64FF00" :
                                j === root.routeCoordinates.length - 1 ? "#FFD43B" : "#00D9FF"
                ctx.fill()
                ctx.strokeStyle = "#FFFFFF"
                ctx.lineWidth = 2
                ctx.stroke()
            }
        }
    }

    DragHandler {
        id: mapDrag
        target: null
        acceptedButtons: Qt.LeftButton
        grabPermissions: PointerHandler.CanTakeOverFromItems |
                         PointerHandler.CanTakeOverFromHandlersOfDifferentType

        onTranslationChanged: {
            root.panOffsetX = translation.x
            root.panOffsetY = translation.y
        }

        onActiveChanged: {
            if (!active)
                root.commitPan()
        }
    }

    WheelHandler {
        id: mapWheel
        target: null
        acceptedDevices: PointerDevice.Mouse
        acceptedModifiers: Qt.NoModifier

        onWheel: function(event) {
            if (event.angleDelta.y === 0)
                return

            var direction = event.angleDelta.y > 0 ? 1 : -1
            root.zoomLevel = root.clamp(root.zoomLevel + direction, 2, 20)
            event.accepted = true
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        anchors.bottomMargin: 8
        width: attributionText.implicitWidth + 16
        height: 22
        color: "#FFFFFF"
        opacity: 0.94
        radius: 2

        Text {
            id: attributionText
            anchors.centerIn: parent
            text: root.attribution
            color: "#5E5E5E"
            font.pixelSize: 10
        }
    }

    Rectangle {
        visible: root.mapStatus !== "READY"
        anchors.centerIn: parent
        width: Math.min(parent.width - 40, statusText.implicitWidth + 40)
        height: 42
        color: "#08111D"
        opacity: 0.92
        radius: 4

        Text {
            id: statusText
            anchors.centerIn: parent
            text: root.mapStatus === "API KEY REQUIRED"
                  ? "GOOGLE MAPS API KEY REQUIRED"
                  : "GOOGLE MAPS: " + root.mapStatus
            color: root.mapStatus === "API KEY REQUIRED" ? "#FFD43B" : "#FFFFFF"
            font.family: "B612 Mono"
            font.pixelSize: 11
        }
    }

    Component.onCompleted: requestSession()
}
