import QtQuick
import QtQuick.Controls
import QtWebEngine

Item {
    id: root

    property string apiKey: ""
    // Planning Core is the sole authority for route geometry. Empty means no route overlay.
    property var routeCoordinates: []
    signal useLegacyMapRequested()
    // SDK is loaded in the embedded WebEngine; keep route updates sourced from Planning Core.
    // Keep this diagnostic-ready map change on the review branch until Windows build and visual checks pass.
    property bool htmlLoaded: false

    function buildHtml() {
        var keyLiteral = JSON.stringify(root.apiKey)
        return [
            "<!doctype html>",
            "<html lang='en'><head><meta charset='utf-8'>",
            "<meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'>",
            "<title>BlueSky PRO · MapLibre GL JS / MapTiler Hybrid v4</title>",
            "<link rel='stylesheet' href='https://unpkg.com/maplibre-gl@6.14.0/dist/maplibre-gl.css'>",
            "<style>",
            "html,body,#map{margin:0;padding:0;width:100%;height:100%;overflow:hidden;background:#071321}body{font-family:Arial,sans-serif}",
            ".controls{position:absolute;z-index:5;top:10px;right:10px;display:flex;gap:6px;flex-wrap:wrap;max-width:calc(100% - 20px);background:rgba(5,10,18,.88);padding:8px;border-radius:4px}",
            ".controls button,.controls select{background:#142638;color:#eaf4ff;border:1px solid #36516b;border-radius:3px;padding:7px 9px;font-size:12px}",
            ".controls button.active{border-color:#32ffff;color:#32ffff}",
            ".diag-panel{display:none;position:absolute;z-index:10;left:10px;right:10px;top:48px;bottom:10px;background:rgba(5,10,18,.96);padding:8px;border:1px solid #36516b;border-radius:4px;box-sizing:border-box}",
            ".diag-panel.open{display:flex;flex-direction:column;gap:6px}",
            ".diag-panel textarea{flex:1;min-height:0;width:100%;resize:none;box-sizing:border-box;background:#071321;color:#d5e7f7;border:1px solid #36516b;font:11px monospace;white-space:pre;overflow:auto}",
            ".status{position:absolute;z-index:5;left:10px;top:10px;color:#d5e7f7;background:rgba(5,10,18,.8);padding:6px 8px;font:11px monospace;border-radius:3px}",
            "</style></head><body>",
            "<div id='map'></div>",
            "<div class='status' id='map-status'>MAPTILER HYBRID V4 · INITIALIZING</div>",
            "<div class='controls'>",
            "<select id='style' aria-label='Map style'><option value='hybrid-v4' selected>HYBRID</option><option value='satellite-v4'>SATELLITE</option><option value='streets-v4'>STREETS</option></select>",
            "<button id='terrain' type='button'>3D TERRAIN: OFF</button>",
            "<button id='labels' class='active' type='button'>LABELS: ON</button>",
            "<button id='perf-snapshot' type='button'>PERF SNAPSHOT</button>",
            "</div>",
            "<div class='diag-panel' id='diag-panel'><div style='display:flex;gap:6px;align-items:center;color:#d5e7f7;font:12px monospace'><span>PERFORMANCE SNAPSHOT · API KEY REDACTED</span><button id='diag-refresh' type='button'>REFRESH</button><button id='diag-select' type='button'>SELECT ALL</button><button id='diag-close' type='button'>CLOSE</button></div><textarea id='diag-output' readonly spellcheck='false' aria-label='Performance diagnostics JSON'></textarea></div>",
            "<script type='module'>",
            "import * as maplibregl from 'https://unpkg.com/maplibre-gl@6.14.0/dist/maplibre-gl.mjs';",
            "const apiKey=" + keyLiteral + ";",
            "const statusEl=document.getElementById('map-status');",
            "if(!apiKey){statusEl.textContent='BLUESKY_MAPTILER_API_KEY NOT SET';}",
            "else{",
            "",
            "const map=new maplibregl.Map({container:'map',style:'https://api.maptiler.com/maps/hybrid-v4/style.json?key='+encodeURIComponent(apiKey),center:[31.95990,49.85842],zoom:6.7,attributionControl:true,fadeDuration:0,refreshExpiredTiles:false,cancelPendingTileRequestsWhileZooming:true,crossSourceCollisions:false,maxPitch:60,maxTileCacheZoomLevels:6,canvasContextAttributes:{antialias:false,preserveDrawingBuffer:false}});",
            "let routeData={type:'FeatureCollection',features:[]};",
            "let routeInputSignature=null;",
            "let labelsVisible=true;",
            "let terrainOn=false;",
            "const perfDiag={moveStarts:0,moveEnds:0,styleChanges:0,styleLoads:0,errors:0,recentErrors:[],routeInputCalls:0,routeInputUnchanged:0,applyRouteCalls:0,sourceCreates:0,sourceSetData:0,routeLayerCreates:0,labelApplyCalls:0,labelLayoutChanges:0,lastIntegrationEvent:null,lastMoveStart:0,lastMoveDurationMs:null,lastStyleStart:0,lastStyleDurationMs:null,addLayerCalls:0,addLayerTotalMs:0,addLayerMaxMs:0,slowAddLayers:[],longTasks:0,longTaskTotalMs:0,longTaskMaxMs:0,recentLongTasks:[]};",
            "function profileAddLayerTarget(target,scope){if(!target||typeof target.addLayer!=='function'||target.__blueskyAddLayerProfiled)return;const original=target.addLayer;try{Object.defineProperty(target,'__blueskyAddLayerProfiled',{value:true,configurable:true});target.addLayer=function(layer,...args){const started=performance.now();const layerId=layer&&layer.id?String(layer.id):'(unknown)';perfDiag.addLayerCalls++;try{return original.apply(this,[layer,...args]);}finally{const durationMs=performance.now()-started;perfDiag.addLayerTotalMs+=durationMs;perfDiag.addLayerMaxMs=Math.max(perfDiag.addLayerMaxMs,durationMs);if(durationMs>=8){perfDiag.slowAddLayers.push({scope:scope,layerId:layerId,durationMs:Math.round(durationMs*10)/10,stack:(new Error()).stack||'',atMs:Math.round(performance.now())});if(perfDiag.slowAddLayers.length>30)perfDiag.slowAddLayers.shift();}}};}catch(e){}};profileAddLayerTarget(map,'map.addLayer');function profileCurrentStyleAddLayer(){profileAddLayerTarget(map.style||map._style,'style.addLayer');}profileCurrentStyleAddLayer();map.on('style.load',profileCurrentStyleAddLayer);const originalSetStyle=map.setStyle;map.setStyle=function(...args){const result=originalSetStyle.apply(this,args);profileCurrentStyleAddLayer();return result;};",
            "const perfDiagnosticsEnabled=new URLSearchParams(window.location.search).has('perf');",
            "if(perfDiagnosticsEnabled&&window.PerformanceObserver){try{new PerformanceObserver(list=>{for(const e of list.getEntries()){perfDiag.longTasks++;perfDiag.longTaskTotalMs+=e.duration;perfDiag.longTaskMaxMs=Math.max(perfDiag.longTaskMaxMs,e.duration);perfDiag.recentLongTasks.push({startTimeMs:Math.round(e.startTime),durationMs:Math.round(e.duration),attribution:(e.attribution||[]).map(a=>({name:a.name,containerType:a.containerType,containerName:a.containerName}))});if(perfDiag.recentLongTasks.length>30)perfDiag.recentLongTasks.shift();}}).observe({type:'longtask',buffered:true});}catch(e){}}",
            "window.blueskyPerfSnapshot=function(){const resources=performance.getEntriesByType('resource');const groups={};resources.forEach(r=>{let u;try{u=new URL(r.name);}catch(e){return;}const host=u.hostname;const kind=/\\.pbf|vector|tiles/i.test(u.pathname)?'tile-or-vector':(/terrain|dem|elevation/i.test(u.pathname)?'terrain-or-elevation':(/style\\.json/i.test(u.pathname)?'style':(/\\.js($|\\?)/i.test(u.pathname)?'javascript':(/\\.css($|\\?)/i.test(u.pathname)?'css':'other'))));const k=host+' / '+kind;if(!groups[k])groups[k]={count:0,bytes:0,totalMs:0,maxMs:0,failedOrOpaque:0};const g=groups[k];g.count++;g.bytes+=r.transferSize||0;g.totalMs+=r.duration||0;g.maxMs=Math.max(g.maxMs,r.duration||0);if(r.responseEnd===0)g.failedOrOpaque++;});const style=map.getStyle()||{};const layers=style.layers||[];const sources=style.sources||{};const layerTypes={};layers.forEach(l=>{layerTypes[l.type]=(layerTypes[l.type]||0)+1;});const terrainSources=Object.keys(sources).filter(id=>sources[id]&&sources[id].type==='raster-dem');const hillshadeLayers=layers.filter(l=>l.type==='hillshade').map(l=>({id:l.id,source:l.source,visibility:(l.layout&&l.layout.visibility)||'visible'}));let terrainSpec=null;try{terrainSpec=typeof map.getTerrain==='function'?map.getTerrain():null;}catch(e){}return {capturedAt:new Date().toISOString(),uptimeMs:Math.round(performance.now()),styleAudit:{sourceCount:Object.keys(sources).length,layerCount:layers.length,layerTypes:layerTypes,sources:Object.keys(sources).map(id=>({id:id,type:sources[id].type||null,url:sources[id].url||null,tiles:sources[id].tiles||[],scheme:sources[id].scheme||null,minzoom:sources[id].minzoom??null,maxzoom:sources[id].maxzoom??null})),rasterLayers:layers.filter(l=>l.type==='raster').map(l=>({id:l.id,source:l.source,sourceLayer:l['source-layer']||null,visibility:(l.layout&&l.layout.visibility)||'visible',paint:l.paint||{}})),terrainSources:terrainSources.map(id=>({id:id,url:sources[id].url||null,tiles:sources[id].tiles||[]})),hillshadeLayers:hillshadeLayers,terrainSpec:terrainSpec,terrainResourceEntries:resources.filter(r=>{try{const u=new URL(r.name);return /terrain-rgb-v2|terrain|dem|elevation/i.test(u.pathname);}catch(e){return false;}}).map(r=>({name:r.name,durationMs:Math.round(r.duration),requestStartMs:Math.round(r.requestStart||0),responseStartMs:Math.round(r.responseStart||0),responseEndMs:Math.round(r.responseEnd||0),transferSize:r.transferSize||0,encodedBodySize:r.encodedBodySize||0,decodedBodySize:r.decodedBodySize||0,nextHopProtocol:r.nextHopProtocol||null})),allTileApiResources:resources.filter(r=>/api\\.maptiler\\.com\\/(tiles|data)\\//i.test(r.name)).map(r=>({name:r.name,durationMs:Math.round(r.duration),requestStartMs:Math.round(r.requestStart||0),responseStartMs:Math.round(r.responseStart||0),responseEndMs:Math.round(r.responseEnd||0),transferSize:r.transferSize||0,encodedBodySize:r.encodedBodySize||0,decodedBodySize:r.decodedBodySize||0,nextHopProtocol:r.nextHopProtocol||null}))},map:{moveStarts:perfDiag.moveStarts,moveEnds:perfDiag.moveEnds,lastMoveDurationMs:perfDiag.lastMoveDurationMs,styleChanges:perfDiag.styleChanges,styleLoads:perfDiag.styleLoads,lastStyleDurationMs:perfDiag.lastStyleDurationMs,errors:perfDiag.errors,recentErrors:perfDiag.recentErrors.slice(-20)},integration:{routeInputCalls:perfDiag.routeInputCalls,routeInputUnchanged:perfDiag.routeInputUnchanged,applyRouteCalls:perfDiag.applyRouteCalls,sourceCreates:perfDiag.sourceCreates,sourceSetData:perfDiag.sourceSetData,routeLayerCreates:perfDiag.routeLayerCreates,labelApplyCalls:perfDiag.labelApplyCalls,labelLayoutChanges:perfDiag.labelLayoutChanges,lastIntegrationEvent:perfDiag.lastIntegrationEvent},addLayerProfiling:{calls:perfDiag.addLayerCalls,totalMs:Math.round(perfDiag.addLayerTotalMs*10)/10,maxMs:Math.round(perfDiag.addLayerMaxMs*10)/10,slowCalls:perfDiag.slowAddLayers},longTasks:{count:perfDiag.longTasks,totalMs:Math.round(perfDiag.longTaskTotalMs),maxMs:Math.round(perfDiag.longTaskMaxMs),recent:perfDiag.recentLongTasks},resources:groups};};",
            "map.on('movestart',()=>{perfDiag.moveStarts++;perfDiag.lastMoveStart=performance.now();performance.mark('bluesky-map-movestart');});",
            "map.on('moveend',()=>{perfDiag.moveEnds++;perfDiag.lastMoveDurationMs=Math.round(performance.now()-perfDiag.lastMoveStart);performance.mark('bluesky-map-moveend');});",
            "map.on('style.load',()=>{perfDiag.styleLoads++;if(perfDiag.lastStyleStart)perfDiag.lastStyleDurationMs=Math.round(performance.now()-perfDiag.lastStyleStart);performance.mark('bluesky-map-style-loaded');});",
            "function setRoute(coords){perfDiag.routeInputCalls++;perfDiag.lastIntegrationEvent={type:'setRoute',atMs:Math.round(performance.now())};",
            "const valid=Array.isArray(coords)?coords.filter(p=>p&&Number.isFinite(Number(p.lon))&&Number.isFinite(Number(p.lat))):[];",
            "const normalized=valid.map(p=>[Number(p.lon),Number(p.lat)]);",
            "const signature=JSON.stringify(normalized);",
            "if(signature===routeInputSignature){perfDiag.routeInputUnchanged++;return;}",
            "routeInputSignature=signature;",
            "routeData={type:'FeatureCollection',features:[]};",
            "if(normalized.length>=2){routeData.features.push({type:'Feature',properties:{kind:'route'},geometry:{type:'LineString',coordinates:normalized}});}",
            "normalized.forEach((p,i)=>routeData.features.push({type:'Feature',properties:{kind:i===0?'start':(i===normalized.length-1?'finish':'waypoint')},geometry:{type:'Point',coordinates:p}}));",
            "if(map.isStyleLoaded())applyRoute();",
            "}",
            "function applyRoute(){perfDiag.applyRouteCalls++;perfDiag.lastIntegrationEvent={type:'applyRoute',atMs:Math.round(performance.now())};",
            "if(!routeData.features.length&&!map.getSource('bluesky-route')&&!map.getLayer('bluesky-route-line'))return;",
            "if(!map.getSource('bluesky-route')){perfDiag.sourceCreates++;map.addSource('bluesky-route',{type:'geojson',data:routeData});}else{perfDiag.sourceSetData++;map.getSource('bluesky-route').setData(routeData);}",
            "if(!map.getLayer('bluesky-route-line')){perfDiag.routeLayerCreates++;map.addLayer({id:'bluesky-route-line',type:'line',source:'bluesky-route',filter:['==',['get','kind'],'route'],layout:{'line-cap':'round','line-join':'round'},paint:{'line-color':'#FF32C8','line-width':3,'line-opacity':0.95}});}",
            "if(!map.getLayer('bluesky-route-points')){perfDiag.routeLayerCreates++;map.addLayer({id:'bluesky-route-points',type:'circle',source:'bluesky-route',filter:['!=',['get','kind'],'route'],paint:{'circle-radius':['match',['get','kind'],'start',7,'finish',7,4],'circle-color':['match',['get','kind'],'start','#64FF00','finish','#FF4658','#32FFFF'],'circle-stroke-color':'#FFFFFF','circle-stroke-width':1.5}});}",
            "}",
            "let labelBaseVisibility={};",
            "function applyLabels(captureBase){perfDiag.labelApplyCalls++;if(!map.isStyleLoaded())return;const layers=(map.getStyle()&&map.getStyle().layers)||[];if(captureBase){labelBaseVisibility={};layers.forEach(l=>{if(l.type==='symbol'&&l.layout&&l.layout['text-field'])labelBaseVisibility[l.id]=l.layout.visibility||'visible';});}layers.forEach(l=>{if(l.type!=='symbol'||!l.layout||!l.layout['text-field'])return;const targetVisibility=labelsVisible?(labelBaseVisibility[l.id]||'visible'):'none';const currentVisibility=l.layout.visibility||'visible';if(currentVisibility!==targetVisibility&&map.getLayer(l.id)){try{map.setLayoutProperty(l.id,'visibility',targetVisibility);perfDiag.labelLayoutChanges++;perfDiag.lastIntegrationEvent={type:'labelLayoutChange',layerId:l.id,atMs:Math.round(performance.now())};}catch(e){perfDiag.recentErrors.push({atMs:Math.round(performance.now()),type:'label-layout-update',message:e&&e.message?String(e.message):String(e),stack:e&&e.stack?String(e.stack).slice(0,4000):null,layerId:l.id});if(perfDiag.recentErrors.length>30)perfDiag.recentErrors.shift();}}});}",
            "window.blueskySetRoute=setRoute;",
            "const diagPanel=document.getElementById('diag-panel');",
            "const diagOutput=document.getElementById('diag-output');",
            "function renderPerfSnapshot(){if(typeof window.blueskyPerfSnapshot!=='function'){diagOutput.value='Diagnostics are not ready.';return;}try{diagOutput.value=JSON.stringify(window.blueskyPerfSnapshot(),(k,v)=>typeof v==='string'&&apiKey?v.split(apiKey).join('[REDACTED]'):v,2);}catch(e){diagOutput.value='Snapshot failed: '+String(e);}}",
            "document.getElementById('perf-snapshot').addEventListener('click',()=>{renderPerfSnapshot();diagPanel.classList.add('open');});",
            "document.getElementById('diag-refresh').addEventListener('click',renderPerfSnapshot);",
            "document.getElementById('diag-select').addEventListener('click',()=>{diagOutput.focus();diagOutput.select();});",
            "document.getElementById('diag-close').addEventListener('click',()=>diagPanel.classList.remove('open'));",
            "map.on('load',()=>{statusEl.textContent='MAPTILER HYBRID V4 · READY';});",
            "map.on('error',e=>{perfDiag.errors++;const err=e&&e.error?e.error:null;const item={atMs:Math.round(performance.now()),type:e&&e.type?String(e.type):'unknown',message:err&&err.message?String(err.message):'Map error without error.message',stack:err&&err.stack?String(err.stack).slice(0,4000):null,sourceId:e&&e.sourceId?String(e.sourceId):null,sourceType:e&&e.sourceType?String(e.sourceType):null,tileId:e&&e.tile&&e.tile.uid!==undefined?String(e.tile.uid):null};perfDiag.recentErrors.push(item);if(perfDiag.recentErrors.length>30)perfDiag.recentErrors.shift();console.warn('MapTiler map error:',item.message);});",
            "function enableTerrain(){if(!map.getSource('bluesky-terrain-dem'))map.addSource('bluesky-terrain-dem',{type:'raster-dem',url:'https://api.maptiler.com/tiles/terrain-rgb-v2/tiles.json?key='+encodeURIComponent(apiKey),tileSize:256});map.setTerrain({source:'bluesky-terrain-dem',exaggeration:1.0});}map.on('style.load',()=>{applyRoute();applyLabels(true);if(terrainOn){try{enableTerrain();}catch(e){console.warn('Terrain unavailable:',e.message);}}});",
            "document.getElementById('style').addEventListener('change',e=>{const id=e.target.value;perfDiag.styleChanges++;perfDiag.lastStyleStart=performance.now();performance.mark('bluesky-map-style-change');statusEl.textContent='MAPTILER '+id.toUpperCase()+' · LOADING';map.setStyle('https://api.maptiler.com/maps/'+id+'/style.json?key='+encodeURIComponent(apiKey),{diff:true});});",
            "document.getElementById('terrain').addEventListener('click',()=>{terrainOn=!terrainOn;try{if(terrainOn){if(map.isStyleLoaded())enableTerrain();map.easeTo({pitch:55,duration:250});}else{map.setTerrain(null);map.easeTo({pitch:0,duration:350});}document.getElementById('terrain').textContent='3D TERRAIN: '+(terrainOn?'ON':'OFF');document.getElementById('terrain').classList.toggle('active',terrainOn);}catch(e){terrainOn=false;statusEl.textContent='3D TERRAIN UNAVAILABLE';console.warn('Terrain toggle failed:',e.message);}});",
            "document.getElementById('labels').addEventListener('click',()=>{labelsVisible=!labelsVisible;applyLabels(false);document.getElementById('labels').textContent='LABELS: '+(labelsVisible?'ON':'OFF');document.getElementById('labels').classList.toggle('active',labelsVisible);});",
            "window.blueskySetRoute(" + JSON.stringify(root.routeCoordinates || []) + ");",
            "}",
            "</script></body></html>"
        ].join("\n")
    }

    function syncRouteCoordinates() {
        if (htmlLoaded && !webMap.loading) {
            var payload = JSON.stringify(root.routeCoordinates || [])
            webMap.runJavaScript("if(window.blueskySetRoute)window.blueskySetRoute(" + payload + ");")
        }
    }

    WebEngineView {
        id: webMap
        anchors.fill: parent
        visible: root.apiKey.length > 0
        backgroundColor: "#071321"
        onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            var safeMessage = String(message)
            if (root.apiKey.length > 0)
                safeMessage = safeMessage.split(root.apiKey).join("[REDACTED]")
            console.warn("MapTiler JS console [level=" + level + ", line=" + lineNumber + ", source=" + sourceID + "]: " + safeMessage)
        }
        onLoadingChanged: function(request) {
            if (request.status === WebEngineView.LoadSucceededStatus) {
                root.htmlLoaded = true
                root.syncRouteCoordinates()
            } else if (request.status === WebEngineView.LoadFailedStatus) {
                console.warn("MapTiler SDK load failed:", request.errorString)
            }
        }
        Component.onCompleted: {
            if (root.apiKey.length > 0)
                loadHtml(root.buildHtml(), "https://api.maptiler.com/")
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
            color: "#FFD43B"
            font.family: "monospace"
            text: "MAPTILER HYBRID V4\n\nSet BLUESKY_MAPTILER_API_KEY in the environment, then restart BlueSky PRO."
        }
    }

    Button {
        z: 20
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 10
        text: "LEGACY MAP"
        onClicked: root.useLegacyMapRequested()
    }

    Connections {
        target: root
        function onRouteCoordinatesChanged() { root.syncRouteCoordinates() }
    }
}
