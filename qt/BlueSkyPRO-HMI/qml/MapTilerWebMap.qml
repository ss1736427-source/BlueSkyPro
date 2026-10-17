import QtQuick
import QtQuick.Controls
import QtWebEngine

Item {
    id: root

    property string apiKey: ""
    property var routeCoordinates: []
    signal useLegacyMapRequested()

    property bool htmlLoaded: false

    function buildHtml() {
        var keyLiteral = JSON.stringify(root.apiKey)
        return [
            "<!doctype html>",
            "<html lang='en'><head><meta charset='utf-8'>",
            "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>",
            "<title>BlueSky PRO MapTiler WebGL Prototype</title>",
            "<link rel='stylesheet' href='https://cdn.maptiler.com/maptiler-sdk-js/v4.2.0/maptiler-sdk.css'>",
            "<script src='https://cdn.maptiler.com/maptiler-sdk-js/v4.2.0/maptiler-sdk.umd.min.js'><\/script>",
            "<style>",
            "html,body,#map{margin:0;width:100%;height:100%;overflow:hidden;background:#071321}",
            ".maplibregl-ctrl-group{border-radius:4px!important;box-shadow:0 2px 8px #0008!important}",
            "#status{position:absolute;left:10px;bottom:10px;z-index:5;padding:5px 8px;background:#071321dd;color:#e5f0fa;font:11px monospace;border:1px solid #31506a;border-radius:3px;pointer-events:none}",
            "#error{display:none;position:absolute;inset:20% 10%;z-index:8;padding:16px;background:#08111dee;color:#ffd43b;font:13px monospace;white-space:pre-wrap}",
            "</style></head><body><div id='map'></div><div id='status'>MAPTILER WEBGL · INITIALIZING</div><div id='error'></div>",
            "<script>",
            "const apiKey=" + keyLiteral + ";",
            "const statusEl=document.getElementById('status');",
            "if(!apiKey){statusEl.textContent='BLUESKY_MAPTILER_API_KEY NOT SET';}",
            "else {",
            "try {",
            "maptilersdk.config.apiKey=apiKey;",
            "const map=window.blueSkyMap=new maptilersdk.Map({container:'map',style:'https://api.maptiler.com/maps/hybrid-v4/style.json?key='+encodeURIComponent(apiKey),center:[37.6176,55.7558],zoom:10,pitch:0,attributionControl:true,navigationControl:true,terrain:false,canvasContextAttributes:{antialias:true}});",
            "map.addControl(new maptilersdk.NavigationControl({visualizePitch:true}),'top-right');",
            "map.on('load',()=>{statusEl.textContent='MAPTILER HYBRID V4 · WEBGL · READY';window.blueSkyMapReady=true;if(window.blueSkyPendingRoute)window.setBlueSkyRoute(window.blueSkyPendingRoute);});",
            "map.on('error',(e)=>{const msg=e&&e.error&&e.error.message?e.error.message:'Map request failed';statusEl.textContent='MAP ERROR · '+msg.slice(0,100);});",
            "window.setBlueSkyTerrain=function(enabled){try{if(enabled){map.enableTerrain();map.easeTo({pitch:62,duration:500});statusEl.textContent='3D TERRAIN · LOADING';}else{map.disableTerrain();map.easeTo({pitch:0,duration:500});statusEl.textContent='MAPTILER HYBRID V4 · 2D';}}catch(e){statusEl.textContent='TERRAIN UNAVAILABLE · '+e.message;}};",
            "window.setBlueSkyRoute=function(coords){window.blueSkyPendingRoute=coords;if(!window.blueSkyMapReady||!Array.isArray(coords))return;const features=coords.filter(p=>p&&Number.isFinite(Number(p.lon))&&Number.isFinite(Number(p.lat))).map(p=>[Number(p.lon),Number(p.lat)]);const data={type:'Feature',properties:{},geometry:{type:'LineString',coordinates:features}};if(map.getSource('bluesky-route')){map.getSource('bluesky-route').setData(data);}else if(features.length>1){map.addSource('bluesky-route',{type:'geojson',data:data});map.addLayer({id:'bluesky-route-line',type:'line',source:'bluesky-route',paint:{'line-color':'#32ffff','line-width':3,'line-opacity':0.95}});}};",
            "window.setBlueSkyRoute(" + JSON.stringify(root.routeCoordinates) + ");",
            "}catch(e){statusEl.textContent='MAP INITIALIZATION ERROR';const box=document.getElementById('error');box.style.display='block';box.textContent=String(e&&e.stack||e);}",
            "}",
            "</script></body></html>"
        ].join("\n")
    }

    WebEngineView {
        id: webMap
        anchors.fill: parent
        visible: root.apiKey.length > 0
        backgroundColor: "#071321"
        onLoadingChanged: function(request) {
            if (request.status === WebEngineView.LoadFailedStatus) {
                console.warn("MapTiler WebEngine HTML load failed:", request.errorString)
            }
        }
        Component.onCompleted: {
            if (root.apiKey.length > 0) {
                loadHtml(root.buildHtml(), "https://api.maptiler.com/")
                root.htmlLoaded = true
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: root.apiKey.length === 0
        color: "#071321"
        Text {
            anchors.centerIn: parent
            width: parent.width - 48
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: "#ffd43b"
            font.family: "monospace"
            text: "MAPTILER WEBGL PROTOTYPE\n\nSet BLUESKY_MAPTILER_API_KEY in the environment, then restart BlueSky PRO."
        }
    }

    Row {
        z: 20
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 10
        spacing: 6

        Button {
            text: "2D"
            onClicked: webMap.runJavaScript("window.setBlueSkyTerrain && window.setBlueSkyTerrain(false)")
        }
        Button {
            text: "3D TERRAIN"
            onClicked: webMap.runJavaScript("window.setBlueSkyTerrain && window.setBlueSkyTerrain(true)")
        }
        Button {
            text: "LEGACY MAP"
            onClicked: root.useLegacyMapRequested()
        }
    }

    Connections {
        target: root
        function onRouteCoordinatesChanged() {
            webMap.runJavaScript("window.setBlueSkyRoute && window.setBlueSkyRoute(" + JSON.stringify(root.routeCoordinates) + ")")
        }
    }
}
