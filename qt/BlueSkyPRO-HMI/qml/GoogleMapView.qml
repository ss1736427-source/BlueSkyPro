import QtQuick
import QtQml

Item {
    id: root

    property real centerLatitude: 55.7558
    property real centerLongitude: 37.6176
    property int zoomLevel: 10
    // Fractional visual zoom is applied immediately to loaded tiles; network
    // requests are rebuilt only when a whole zoom level is crossed.
    property real visualZoomScale: 1.0
    // Side panels overlay this full-width map view.
    property real leftPanelWidth: 0
    property real rightPanelWidth: 0
    // Provider selection is user-controlled. MapTiler Hybrid v4 is the default
    // when its runtime key exists; Yandex remains available as an explicit fallback.
    property string selectedMapProvider:
        (typeof mapTilerApiKey !== "undefined" && mapTilerApiKey.length > 0)
        ? "MAPTILER_HYBRID" : "YANDEX"
    property bool providerMenuOpen: false
    readonly property bool mapTilerKeyAvailable:
        typeof mapTilerApiKey !== "undefined" && mapTilerApiKey.length > 0
    readonly property bool yandexKeyAvailable:
        typeof yandexMapsApiKey !== "undefined" && yandexMapsApiKey.length > 0
    readonly property bool useMapTiler:
        selectedMapProvider.indexOf("MAPTILER") === 0 && mapTilerKeyAvailable
    readonly property bool useCartoDark:
        selectedMapProvider === "CARTO_DARK" &&
        typeof cartoApiKey !== "undefined" && cartoApiKey.length > 0
    readonly property string selectedProviderLabel:
        selectedMapProvider === "MAPTILER_HYBRID" ? "MAPTILER HYBRID V4" :
        selectedMapProvider === "MAPTILER_HYBRID_DARK" ? "MAPTILER HYBRID DARK" :
        selectedMapProvider === "CARTO_DARK" ? "CARTO DARK" : "YANDEX MAPS"
    property string attribution:
        useMapTiler ? "© MapTiler © OpenStreetMap contributors" :
        (useCartoDark ? "© OpenStreetMap contributors, © CARTO" : "© Яндекс")
    property string mapStatus: "INITIALIZING"
    // No preview route: only verified Planning Core geometry may be rendered.
    property var routeCoordinates: []

    readonly property int tileSize: 256
    property real panOffsetX: 0
    property real panOffsetY: 0
    property bool committingPan: false
    property var tiles: []
    // Keep the last complete tile set visible while the next viewport loads.
    property var fallbackTiles: []
    property var stableTiles: []
    property int loadedTileCount: 0
    property int failedTileCount: 0

    signal viewChanged(real latitude, real longitude, int zoom)

    function clamp(value, minimum, maximum) {
        return Math.max(minimum, Math.min(maximum, value))
    }

    function worldSize() {
        return tileSize * Math.pow(2, zoomLevel)
    }

    function applySmoothZoom(delta) {
        if (!delta)
            return

        // One wheel notch corresponds to a quarter zoom level. Scaling is
        // continuous; the discrete tile level changes only at exact 2x/0.5x.
        visualZoomScale *= Math.pow(2, delta / 120.0 * 0.25)
        visualZoomScale = clamp(visualZoomScale, 0.5, 2.0)

        if (visualZoomScale >= 2.0 && zoomLevel < 20) {
            zoomLevel += 1
            visualZoomScale = 1.0
        } else if (visualZoomScale <= 0.5 && zoomLevel > 2) {
            zoomLevel -= 1
            visualZoomScale = 1.0
        }
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

        // Preserve the current tile layer until the replacement viewport is
        // fully ready; this avoids clearing the map during network fetches.
        if (stableTiles.length > 0)
            fallbackTiles = stableTiles

        // Cancel queued requests from the previous viewport before scheduling
        // the current visible tiles. In-flight requests may still populate cache.
        tileCacheManager.beginViewUpdate()
        loadedTileCount = 0
        failedTileCount = 0
        if ((selectedMapProvider.indexOf("MAPTILER") === 0 && !mapTilerKeyAvailable) ||
                (selectedMapProvider === "CARTO_DARK" &&
                 (typeof cartoApiKey === "undefined" || cartoApiKey.length === 0)) ||
                (selectedMapProvider === "YANDEX" && !yandexKeyAvailable)) {
            mapStatus = "API KEY REQUIRED"
            // Keep fallback tiles visible even when a provider key is missing.
            tiles = []
            return
        }

        mapStatus = "LOADING"

        var cx = longitudeToWorld(centerLongitude)
        var cy = latitudeToWorld(centerLatitude)
        var tileCount = Math.pow(2, zoomLevel)
        // Request a two-tile overscan around the viewport. During continuous
        // visual zoom (0.5x..2x) and an active drag, the visible bounds extend
        // beyond the nominal viewport; overscan prevents exposed background
        // while the next integer zoom level is loading.
        var overscanTiles = 2
        var firstX = Math.floor((cx - width / 2) / tileSize) - overscanTiles
        var lastX = Math.floor((cx + width / 2) / tileSize) + overscanTiles
        var firstY = Math.floor((cy - height / 2) / tileSize) - overscanTiles
        var lastY = Math.floor((cy + height / 2) / tileSize) + overscanTiles
        var result = []

        for (var ty = firstY; ty <= lastY; ++ty) {
            if (ty < 0 || ty >= tileCount)
                continue

            for (var tx = firstX; tx <= lastX; ++tx) {
                var wrappedX = ((tx % tileCount) + tileCount) % tileCount
                result.push({
                    tx: tx,
                    ty: ty,
                    tileZoom: zoomLevel,
                    x: tx * tileSize - cx + width / 2 + panOffsetX,
                    y: ty * tileSize - cy + height / 2 + panOffsetY,
                    key: (useMapTiler ? "maptiler/" + selectedMapProvider.toLowerCase() + "/" :
                          (useCartoDark ? "carto/dark_all/" : "yandex/future_map/web_mercator/")) +
                         zoomLevel + "/" + wrappedX + "/" + ty,
                    url: useMapTiler
                         ? "https://api.maptiler.com/maps/" +
                           (selectedMapProvider === "MAPTILER_HYBRID_DARK" ? "hybrid-v4-dark" : "hybrid-v4") +
                           "/256/" + zoomLevel + "/" + wrappedX + "/" + ty + ".png?key=" +
                           encodeURIComponent(mapTilerApiKey)
                         : (useCartoDark
                            ? "https://basemaps.cartocdn.com/rastertiles/dark_all/" +
                              zoomLevel + "/" + wrappedX + "/" + ty + ".png?key=" +
                              encodeURIComponent(cartoApiKey)
                            : "https://tiles.api-maps.yandex.ru/v1/tiles/?x=" +
                              wrappedX + "&y=" + ty + "&z=" + zoomLevel +
                              "&lang=en_US&l=map&maptype=future_map&projection=web_mercator&apikey=" +
                              encodeURIComponent(yandexMapsApiKey)),
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
        var nextLongitude = worldToLongitude(centerX)
        var nextLatitude = clamp(worldToLatitude(centerY), -85.0, 85.0)

        // Avoid rebuilding twice from the two center property notifications.
        committingPan = true
        panOffsetX = 0
        panOffsetY = 0
        centerLongitude = nextLongitude
        centerLatitude = nextLatitude
        committingPan = false

        rebuildTiles()
        routeCanvas.requestPaint()
        viewChanged(centerLatitude, centerLongitude, zoomLevel)
    }

    onWidthChanged: rebuildTiles()
    onHeightChanged: rebuildTiles()
    onRouteCoordinatesChanged: routeCanvas.requestPaint()
    onCenterLatitudeChanged: { if (!committingPan) rebuildTiles(); routeCanvas.requestPaint() }
    onCenterLongitudeChanged: { if (!committingPan) rebuildTiles(); routeCanvas.requestPaint() }
    onZoomLevelChanged: { rebuildTiles(); routeCanvas.requestPaint() }
    onPanOffsetXChanged: routeCanvas.requestPaint()
    onPanOffsetYChanged: routeCanvas.requestPaint()

    Rectangle {
        anchors.fill: parent
        color: "#071321"
    }

    // Previous completed viewport stays underneath the incoming tile set.
    // It is deliberately not transformed by the new fractional zoom, so it
    // serves as a stable visual fallback until the new imagery is ready.
    Repeater {
        model: root.fallbackTiles

        delegate: Image {
            // Reproject the retained tile from its own zoom level into the
            // current world coordinate system. Keeping stale screen-space x/y
            // coordinates caused the cross-shaped gaps after pan/zoom.
            readonly property real retainedScale: Math.pow(2, root.zoomLevel - modelData.tileZoom)
            readonly property real retainedWorldSize: root.tileSize * Math.pow(2, root.zoomLevel)
            readonly property real retainedCenterX: root.longitudeToWorld(root.centerLongitude)
            readonly property real retainedCenterY: root.latitudeToWorld(root.centerLatitude)
            readonly property real retainedTileX: modelData.tx * root.tileSize * retainedScale
            readonly property real retainedTileY: modelData.ty * root.tileSize * retainedScale
            x: retainedTileX - retainedCenterX + root.width / 2 + root.panOffsetX
            y: retainedTileY - retainedCenterY + root.height / 2 + root.panOffsetY
            width: root.tileSize * retainedScale
            height: root.tileSize * retainedScale
            source: modelData.source
            asynchronous: true
            cache: modelData.source.indexOf("data:") !== 0
            fillMode: Image.Stretch
            smooth: true
        }
    }

    Repeater {
        model: root.tiles

        delegate: Image {
            x: root.width / 2 + (modelData.x + root.panOffsetX - root.width / 2) * root.visualZoomScale
            y: root.height / 2 + (modelData.y + root.panOffsetY - root.height / 2) * root.visualZoomScale
            width: root.tileSize * root.visualZoomScale
            height: root.tileSize * root.visualZoomScale
            source: modelData.source
            asynchronous: true
            retainWhileLoading: true
            // Do not retain no-store/data-URL tiles in the Qt Quick image cache.
            cache: modelData.source.indexOf("data:") !== 0
            fillMode: Image.Stretch
            property int lastStatus: Image.Null

            onStatusChanged: {
                if (status === lastStatus)
                    return
                lastStatus = status

                if (status === Image.Ready) {
                    root.loadedTileCount++
                    if (root.loadedTileCount === root.tiles.length && root.failedTileCount === 0) {
                        root.mapStatus = "READY"
                        root.stableTiles = root.tiles
                        root.fallbackTiles = []
                    }
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
        opacity: (root.useMapTiler || root.useCartoDark) ? 0.04 : 0.58
        visible: true
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

        transform: Scale {
            origin.x: routeCanvas.width / 2
            origin.y: routeCanvas.height / 2
            xScale: root.visualZoomScale
            yScale: root.visualZoomScale
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
        blocking: true

        onWheel: function(event) {
            // The side panels are siblings layered above this full-width map.
            // Consume their wheel events without changing map zoom.
            if (event.x < root.leftPanelWidth ||
                    event.x >= root.width - root.rightPanelWidth) {
                event.accepted = true
                return
            }
            if (event.angleDelta.y === 0) {
                event.accepted = true
                return
            }

            root.applySmoothZoom(event.angleDelta.y)
            event.accepted = true
        }
    }

    // Basemap selector: the providers remain independently selectable.
    Rectangle {
        id: providerSelector
        z: 31
        anchors.left: parent.left
        anchors.leftMargin: root.leftPanelWidth + 12
        anchors.top: parent.top
        anchors.topMargin: 12
        width: Math.max(148, providerLabel.implicitWidth + 28)
        height: 34
        radius: 4
        color: "#08111D"
        border.color: "#31506A"
        border.width: 1

        Text {
            id: providerLabel
            anchors.centerIn: parent
            text: root.selectedProviderLabel + "  ▾"
            color: "#E5F0FA"
            font.pixelSize: 10
            font.bold: true
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.providerMenuOpen = !root.providerMenuOpen
        }
    }

    Rectangle {
        id: providerMenu
        z: 32
        visible: root.providerMenuOpen
        anchors.left: providerSelector.left
        anchors.top: providerSelector.bottom
        anchors.topMargin: 4
        width: 206
        height: providerOptions.implicitHeight + 12
        radius: 4
        color: "#08111D"
        border.color: "#31506A"
        border.width: 1

        Column {
            id: providerOptions
            anchors.fill: parent
            anchors.margins: 6
            spacing: 2

            Repeater {
                model: [
                    { id: "MAPTILER_HYBRID", label: "MapTiler Hybrid v4" },
                    { id: "MAPTILER_HYBRID_DARK", label: "MapTiler Hybrid v4 Dark" },
                    { id: "YANDEX", label: "Яндекс Карты" }
                ]
                delegate: Rectangle {
                    required property var modelData
                    width: providerOptions.width
                    height: 30
                    radius: 3
                    color: root.selectedMapProvider === modelData.id ? "#173A53" : "transparent"
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.label
                        color: "#E5F0FA"
                        font.pixelSize: 11
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.selectedMapProvider = modelData.id
                            root.providerMenuOpen = false
                            root.rebuildTiles()
                        }
                    }
                }
            }
        }
    }

    // Compact map zoom controls stay inside the visible map workspace,
    // clear of any side panel layered over this full-width view.
    Rectangle {
        id: zoomControls
        z: 30
        anchors.right: parent.right
        anchors.rightMargin: root.rightPanelWidth + 12
        anchors.top: parent.top
        anchors.topMargin: 12
        width: 34
        height: 68
        radius: 4
        color: "#08111D"
        border.color: "#31506A"
        border.width: 1

        Rectangle {
            width: parent.width - 2
            height: 1
            x: 1
            y: parent.height / 2
            color: "#31506A"
        }

        Text {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height / 2
            text: "+"
            color: root.zoomLevel < 20 ? "#E5F0FA" : "#607386"
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height / 2
            enabled: root.zoomLevel < 20
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: root.applySmoothZoom(480)
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height / 2
            text: "−"
            color: root.zoomLevel > 2 ? "#E5F0FA" : "#607386"
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height / 2
            enabled: root.zoomLevel > 2
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: root.applySmoothZoom(-480)
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
                  ? root.selectedProviderLabel + ": API KEY REQUIRED"
                  : root.attribution.toUpperCase() + ": " + root.mapStatus
            color: "#FFD43B"
            font.family: "B612 Mono"
            font.pixelSize: 11
        }
    }

    Connections {
        target: tileCacheManager

        function onTileReady(key, fileUrl) {
            // Tile objects are immutable snapshots copied into tiles,
            // stableTiles and fallbackTiles. Update every live snapshot so a
            // retained viewport never contains empty-source placeholders.
            function withReadySource(list) {
                var updated = list.slice()
                var changed = false
                for (var i = 0; i < updated.length; ++i) {
                    if (updated[i].key === key) {
                        var item = Object.assign({}, updated[i])
                        item.source = fileUrl
                        updated[i] = item
                        changed = true
                    }
                }
                return { list: updated, changed: changed }
            }

            var current = withReadySource(root.tiles)
            if (current.changed)
                root.tiles = current.list

            var stable = withReadySource(root.stableTiles)
            if (stable.changed)
                root.stableTiles = stable.list

            var fallback = withReadySource(root.fallbackTiles)
            if (fallback.changed)
                root.fallbackTiles = fallback.list
        }

        function onTileFailed(key) {
            root.mapStatus = "TILE LOAD ERROR"
        }
    }

    Component.onCompleted: rebuildTiles()
}
