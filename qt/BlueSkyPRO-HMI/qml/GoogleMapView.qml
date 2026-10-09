import QtQuick
import QtQml

Item {
    id: root

    property real centerLatitude: 55.7558
    property real centerLongitude: 37.6176
    property int zoomLevel: 10
    readonly property bool useMapTiler: typeof mapTilerApiKey !== "undefined" && mapTilerApiKey.length > 0
    property string attribution: useMapTiler ? "MapTiler" : "Yandex Maps"
    property string mapStatus: "INITIALIZING"
    // No preview route: only verified Planning Core geometry may be rendered.
    property var routeCoordinates: []

    readonly property int tileSize: 256
    property real panOffsetX: 0
    property real panOffsetY: 0
    property var tiles: []
    property int loadedTileCount: 0
    property int failedTileCount: 0

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
        if (width <= 0 || height <= 0)
            return

        loadedTileCount = 0
        failedTileCount = 0
        if (!useMapTiler && yandexMapsApiKey.length === 0) {
            mapStatus = "API KEY REQUIRED"
            tiles = []
            return
        }

        mapStatus = "LOADING"

        var cx = longitudeToWorld(centerLongitude)
        var cy = latitudeToWorld(centerLatitude)
        var tileCount = Math.pow(2, zoomLevel)
        var firstX = Math.floor((cx - width / 2) / tileSize)
        var lastX = Math.floor((cx + width / 2) / tileSize)
        var firstY = Math.floor((cy - height / 2) / tileSize)
        var lastY = Math.floor((cy + height / 2) / tileSize)
        var result = []

        for (var ty = firstY; ty <= lastY; ++ty) {
            if (ty < 0 || ty >= tileCount)
                continue

            for (var tx = firstX; tx <= lastX; ++tx) {
                var wrappedX = ((tx % tileCount) + tileCount) % tileCount
                result.push({
                    tx: tx,
                    ty: ty,
                    x: tx * tileSize - cx + width / 2 + panOffsetX,
                    y: ty * tileSize - cy + height / 2 + panOffsetY,
                    key: (useMapTiler ? "maptiler/dataviz-dark/" : "yandex/future_map/web_mercator/") +
                         zoomLevel + "/" + wrappedX + "/" + ty,
                    url: useMapTiler
                         ? "https://api.maptiler.com/maps/dataviz-dark/256/" +
                           zoomLevel + "/" + wrappedX + "/" + ty + ".png?key=" +
                           encodeURIComponent(mapTilerApiKey)
                         : "https://tiles.api-maps.yandex.ru/v1/tiles/?x=" +
                           wrappedX + "&y=" + ty + "&z=" + zoomLevel +
                           "&lang=en_US&l=map&maptype=future_map&projection=web_mercator&apikey=" +
                           encodeURIComponent(yandexMapsApiKey),
                    source: ""
                })
            }
        }

        // Keep the visible viewport priority order. The C++ tile cache
        // manager uses this order to fill the persistent cache center-out.
        var viewportCenterX = width / 2
        var viewportCenterY = height / 2
        for (var i = 0; i < result.length; ++i) {
            var dx = result[i].x + tileSize / 2 - viewportCenterX
            var dy = result[i].y + tileSize / 2 - viewportCenterY
            result[i].priority = dx * dx + dy * dy
        }
        result.sort(function(a, b) {
            return a.priority - b.priority
        })

        // Only now enqueue requests, so the cache manager receives the
        // visible tiles in center-out priority order.
        for (var j = 0; j < result.length; ++j) {
            result[j].source = tileCacheManager.requestTile(
                result[j].key, result[j].url)
        }

        tiles = result
    }

    function commitPan() {
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
    onRouteCoordinatesChanged: routeCanvas.requestPaint()
    onCenterLatitudeChanged: { rebuildTiles(); routeCanvas.requestPaint() }
    onCenterLongitudeChanged: { rebuildTiles(); routeCanvas.requestPaint() }
    onZoomLevelChanged: { rebuildTiles(); routeCanvas.requestPaint() }
    onPanOffsetXChanged: routeCanvas.requestPaint()
    onPanOffsetYChanged: routeCanvas.requestPaint()

    Rectangle {
        anchors.fill: parent
        color: "#E8EEF2"
    }

    Repeater {
        model: root.tiles

        delegate: Image {
            x: modelData.x + root.panOffsetX
            y: modelData.y + root.panOffsetY
            width: root.tileSize
            height: root.tileSize
            source: modelData.source
            asynchronous: true
            retainWhileLoading: true
            cache: true
            fillMode: Image.Stretch
            property int lastStatus: Image.Null

            onStatusChanged: {
                if (status === lastStatus)
                    return
                lastStatus = status

                if (status === Image.Ready) {
                    root.loadedTileCount++
                    if (root.loadedTileCount === root.tiles.length && root.failedTileCount === 0)
                        root.mapStatus = "READY"
                } else if (status === Image.Error) {
                    root.failedTileCount++
                    root.mapStatus = "TILE LOAD ERROR"
                }
            }

            smooth: true
        }
    }

    // Dark Aviation treatment: keep provider tiles intact and apply a restrained
    // navy tint above the basemap, below the authoritative route geometry.
    Rectangle {
        id: aviationMapTint
        anchors.fill: parent
        z: 10
        color: "#071321"
        opacity: root.useMapTiler ? 0.12 : 0.38
        enabled: false
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

            ctx.strokeStyle = "#FF32C8"
            ctx.lineWidth = 3
            ctx.lineJoin = "round"
            ctx.stroke()

            for (var j = 0; j < root.routeCoordinates.length; ++j) {
                var marker = pointForCoordinate(root.routeCoordinates[j])
                ctx.beginPath()
                if (j === 0 || j === root.routeCoordinates.length - 1) {
                    ctx.arc(marker.x, marker.y, 7, 0, Math.PI * 2)
                    ctx.fillStyle = j === 0 ? "#64FF00" : "#FF4658"
                    ctx.fill()
                    ctx.strokeStyle = j === 0 ? "#B7FF8A" : "#FFB0B8"
                    ctx.lineWidth = 2
                    ctx.stroke()
                } else {
                    // Aviation-chart waypoint diamond, similar to the reference UI.
                    ctx.moveTo(marker.x, marker.y - 6)
                    ctx.lineTo(marker.x + 6, marker.y)
                    ctx.lineTo(marker.x, marker.y + 6)
                    ctx.lineTo(marker.x - 6, marker.y)
                    ctx.closePath()
                    ctx.fillStyle = "#FF9F43"
                    ctx.fill()
                    ctx.strokeStyle = "#FFD3A3"
                    ctx.lineWidth = 1
                    ctx.stroke()
                }
            }
        }
    }

    DragHandler {
        id: mapDrag
        target: null
        acceptedButtons: Qt.LeftButton

        onActiveTranslationChanged: {
            root.panOffsetX = activeTranslation.x
            root.panOffsetY = activeTranslation.y
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
            text: (!root.useMapTiler && yandexMapsApiKey.length === 0)
                  ? "MAP API KEY REQUIRED"
                  : root.attribution.toUpperCase() + ": " + root.mapStatus
            color: "#FFD43B"
            font.family: "B612 Mono"
            font.pixelSize: 11
        }
    }

    Connections {
        target: tileCacheManager

        function onTileReady(key, fileUrl) {
            var updated = root.tiles.slice()
            var changed = false

            for (var i = 0; i < updated.length; ++i) {
                if (updated[i].key === key) {
                    updated[i].source = fileUrl
                    changed = true
                    break
                }
            }

            if (changed)
                root.tiles = updated
        }

        function onTileFailed(key) {
            root.mapStatus = "TILE LOAD ERROR"
        }
    }

    Component.onCompleted: rebuildTiles()
}
