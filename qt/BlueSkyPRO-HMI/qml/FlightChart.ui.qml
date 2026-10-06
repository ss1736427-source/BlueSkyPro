import QtQuick
import QtWebEngine

Item {
    id: root

    // BlueSky PRO — live Yandex geographic map workspace.
    property color bg: "#050A12"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color muted: "#7F7F7F"
    property color divider: "#1B2A3A"
    property color cyan: "#32FFFF"
    property bool missionVisible: true
    property bool manualCreationMode: false
    property bool manualCompositionComplete: false

    // Retained for MainContent state compatibility. Geographic pan/zoom is owned
    // by Yandex Maps once the live map is loaded.
    property bool useExternalMapState: false
    property real mapPanX: 0
    property real mapPanY: 0
    property real mapZoom: 1.0
    signal mapViewChangeRequested(real panX, real panY, real zoom)
    signal manualCompositionCompleted()
    signal mapDoubleClicked()

    property string yandexApiKey: yandexMapsApiKey
    property bool mapReady: false
    property string mapStatus: "OFFLINE"

    // Geographic data contract for the planning engine.
    // Coordinates use Yandex's default [latitude, longitude] order.
    property var routeCoordinates: [
        [55.690, 37.540],
        [55.735, 37.585],
        [55.775, 37.650],
        [55.735, 37.715]
    ]
    property var restrictionZones: []
    property var notamItems: []

    function runMapScript(script) {
        if (!mapReady)
            return
        mapView.runJavaScript(script)
    }

    function setMapType(type) {
        runMapScript("setMapType(" + JSON.stringify(type) + ");")
    }

    function setRoute(points) {
        runMapScript("setRoute(" + JSON.stringify(points) + ");")
    }

    function setRestrictions(zones) {
        runMapScript("setRestrictions(" + JSON.stringify(zones) + ");")
    }

    function setNotams(items) {
        runMapScript("setNotams(" + JSON.stringify(items) + ");")
    }

    function setLayerVisible(layer, visible) {
        runMapScript("setLayerVisible(" + JSON.stringify(layer) + "," + (visible ? "true" : "false") + ");")
    }

    function loadYandexMap() {
        if (!root.yandexApiKey) {
            mapView.loadHtml(noKeyHtml(), "https://yandex.com/")
            return
        }
        mapView.loadHtml(yandexMapHtml(), "https://yandex.com/")
    }

    function noKeyHtml() {
        return "<!doctype html><html><head><meta charset='utf-8'>" +
               "<style>html,body{margin:0;width:100%;height:100%;background:#050A12;color:#BFBFBF;" +
               "font-family:Arial,sans-serif}main{height:100%;display:flex;align-items:center;" +
               "justify-content:center;text-align:center}strong{color:#64FF00}</style></head>" +
               "<body><main><div><strong>YANDEX MAPS</strong><br><br>" +
               "API key is not configured.<br>" +
               "Set BLUESKY_YANDEX_MAPS_API_KEY and restart BlueSky PRO.</div></main></body></html>"
    }

    function yandexMapHtml() {
        var key = encodeURIComponent(root.yandexApiKey)
        return "<!doctype html><html><head><meta charset='utf-8'>" +
               "<meta name='viewport' content='width=device-width,initial-scale=1'>" +
               "<style>html,body,#map{width:100%;height:100%;margin:0;padding:0;overflow:hidden}" +
               "body{background:#050A12}</style>" +
               "<script src='https://api-maps.yandex.ru/2.1/?apikey=" + key +
               "&lang=ru_RU' type='text/javascript'></script></head><body>" +
               "<div id='map'></div><script>" +
               "var map, routeObjects=[], restrictionObjects=[], notamObjects=[];" +
               "function clearObjects(list){for(var i=0;i<list.length;i++)map.geoObjects.remove(list[i]);list.length=0;}" +
               "function setMapType(type){if(map)map.setType(type);}" +
               "function setLayerVisible(layer,visible){" +
               "var list=layer==='route'?routeObjects:(layer==='restrictions'?restrictionObjects:notamObjects);" +
               "for(var i=0;i<list.length;i++)list[i].options.set('visible',visible);" +
               "}" +
               "function setRoute(points){" +
               "clearObjects(routeObjects);if(!points||points.length<2)return;" +
               "var line=new ymaps.Polyline(points,{},{" +
               "strokeColor:'#32FFFF',strokeWidth:3,strokeOpacity:0.9,geodesic:true});" +
               "routeObjects.push(line);map.geoObjects.add(line);" +
               "routeObjects.push(new ymaps.Placemark(points[0],{balloonContent:'СТАРТ'},{preset:'islands#greenCircleDotIcon'}));" +
               "routeObjects.push(new ymaps.Placemark(points[points.length-1],{balloonContent:'ФИНИШ'},{preset:'islands#yellowCircleDotIcon'}));" +
               "map.geoObjects.add(routeObjects[1]);map.geoObjects.add(routeObjects[2]);" +
               "}" +
               "function setRestrictions(zones){" +
               "clearObjects(restrictionObjects);if(!zones)return;" +
               "for(var i=0;i<zones.length;i++){var z=zones[i];var o=new ymaps.Polygon(z.coordinates,{balloonContent:z.name||'ЗАПРЕТНАЯ ЗОНА'},{fillColor:'#FF4D5A55',strokeColor:'#FF4D5A',strokeWidth:2});restrictionObjects.push(o);map.geoObjects.add(o);}" +
               "}" +
               "function setNotams(items){" +
               "clearObjects(notamObjects);if(!items)return;" +
               "for(var i=0;i<items.length;i++){var n=items[i];var o=new ymaps.Placemark(n.position,{balloonContent:n.text||'NOTAM'},{preset:'islands#redCircleDotIcon'});notamObjects.push(o);map.geoObjects.add(o);}" +
               "}" +
               "ymaps.ready(function(){" +
               "map=new ymaps.Map('map',{center:[55.7558,37.6176],zoom:10,type:'yandex#map'," +
               "controls:['zoomControl','typeSelector','fullscreenControl']},{searchControlProvider:'yandex#search'});" +
               "window.blueskyMapReady=true;" +
               "});</script></body></html>"
    }

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    WebEngineView {
        id: mapView
        anchors.fill: parent
        z: 0
        backgroundColor: root.bg
        settings.javascriptEnabled: true
        settings.localContentCanAccessRemoteUrls: true
        settings.errorPageEnabled: true

        Component.onCompleted: root.loadYandexMap()

        onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebEngineView.LoadStartedStatus) {
                root.mapReady = false
                root.mapStatus = "LOADING"
            } else if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
                root.mapStatus = "LOADING"
                mapReadyProbe.start()
            } else if (loadRequest.status === WebEngineView.LoadFailedStatus) {
                root.mapReady = false
                root.mapStatus = "ERROR"
                mapReadyProbe.stop()
                console.log("Yandex Maps load failed:", loadRequest.errorString)
            }
        }

        onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            console.log("Yandex Maps:", message, "line", lineNumber, sourceID)
        }
    }

    Timer {
        id: mapReadyProbe
        interval: 250
        repeat: true
        onTriggered: {
            mapView.runJavaScript("window.blueskyMapReady === true", function(result) {
                if (result === true) {
                    stop()
                    root.mapReady = true
                    root.mapStatus = "ONLINE"
                    root.setRoute(root.routeCoordinates)
                    root.setRestrictions(root.restrictionZones)
                    root.setNotams(root.notamItems)
                }
            })
        }
    }

    // Mission state overlay remains native QML and sits above the geographic map.
    Text {
        visible: true
        x: 18
        y: 16
        text: "FLIGHT CHART"
        color: root.secondary
        font.family: "B612"
        font.pixelSize: 14
        font.bold: true
        z: 20
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
        z: 20

        Text {
            anchors.centerIn: parent
            text: root.mapStatus === "ONLINE" ? "YANDEX MAP" : "YANDEX MAP · " + root.mapStatus
            color: "#64FF00"
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
        opacity: 0.92
        z: 20

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
        z: 20

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
        visible: !root.missionVisible && !root.manualCreationMode
        anchors.centerIn: parent
        text: "НЕТ АКТИВНОЙ МИССИИ"
        color: root.muted
        font.family: "B612 Mono"
        font.pixelSize: 12
        z: 30
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
        z: 30
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
