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
               "&lang=en_RU' type='text/javascript'></script></head><body>" +
               "<div id='map'></div><script>" +
               "ymaps.ready(function(){" +
               "var map=new ymaps.Map('map',{center:[37.6176,55.7558],zoom:10,type:'yandex#map'," +
               "controls:['zoomControl','typeSelector','fullscreenControl']},{searchControlProvider:'yandex#search'});" +
               "var demoRoute=[[37.540,55.690],[37.585,55.735],[37.650,55.775],[37.715,55.735]];" +
               "var line=new ymaps.Polyline(demoRoute,{},{" +
               "strokeColor:'#32FFFF',strokeWidth:3,strokeOpacity:0.9,geodesic:true});" +
               "map.geoObjects.add(line);" +
               "map.geoObjects.add(new ymaps.Placemark(demoRoute[0],{balloonContent:'DEMO START'},{" +
               "preset:'islands#greenCircleDotIcon'}));" +
               "map.geoObjects.add(new ymaps.Placemark(demoRoute[demoRoute.length-1],{balloonContent:'DEMO FINISH'},{" +
               "preset:'islands#yellowCircleDotIcon'}));" +
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
            if (loadRequest.status === WebEngineView.LoadFailedStatus)
                console.log("Yandex Maps load failed:", loadRequest.errorString)
        }

        onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            console.log("Yandex Maps:", message, "line", lineNumber, sourceID)
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
            text: "YANDEX MAP"
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
