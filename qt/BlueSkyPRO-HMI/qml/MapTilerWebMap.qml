import QtQuick
import QtQuick.Controls
import QtWebEngine
import QtWebChannel

Item {
    id: root

    property string apiKey: ""
    // Horizontal bounds of the side panels overlaying the full map viewport.
    property real leftPanelWidth: 0
    property real rightPanelWidth: 0
    // Planning Core is the sole authority for route geometry. Empty means no route overlay.
    property var routeCoordinates: []
    property var manualTemplateIds: []
    // Normalized source records are display-only; unknown altitude datum is never inferred.
    property var notamRecords: (typeof faaNmsClient !== "undefined" && faaNmsClient !== null) ? faaNmsClient.notamRecords : []
    property string notamSnapshotId: (typeof faaNmsClient !== "undefined" && faaNmsClient !== null) ? faaNmsClient.notamSnapshotId : ""
    signal useLegacyMapRequested()
    // Use the module CDN as the document origin so MapLibre v6 can create its CDN-derived worker from the same origin in Qt WebEngine.
    // Style switching uses a full style replacement to avoid diff-update incompatibilities between MapTiler styles.
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
            ".notam-boundaries{display:none;position:absolute;z-index:7;left:10px;top:48px;width:min(330px,calc(100% - 20px));max-height:42%;overflow:auto;box-sizing:border-box;background:rgba(5,10,18,.94);border:1px solid #8c6a2a;border-radius:4px;padding:8px;color:#eaf4ff;font:11px Arial,sans-serif}",
            ".notam-boundaries.open{display:block}.notam-boundaries h3{margin:0 0 6px;font-size:12px;color:#ffc45c}.notam-boundary{padding:6px 0;border-top:1px solid #354457;white-space:normal;overflow-wrap:anywhere}.notam-boundary b{color:#ffc45c}.notam-boundary .datum{color:#a9c3d9}",
            "</style></head><body>",
            "<div id='map'></div>",
            "<div id='geometry-controls' style='position:absolute;z-index:8;left:10px;right:10px;bottom:10px;display:none;gap:6px;align-items:center;flex-wrap:wrap;background:rgba(5,10,18,.94);padding:8px;border:1px solid #36516b;color:#eaf4ff;font:11px monospace'><button id='geometry-origin' type='button'>SET TAKEOFF ORIGIN</button><select id='geometry-template'></select><label for='geometry-agl'>TARGET AGL m</label><input id='geometry-agl' type='number' min='20' max='80' step='1' value='40' style='width:66px;background:#142638;color:#eaf4ff;border:1px solid #36516b;padding:7px 5px;font:12px monospace'><button id='geometry-start' type='button'>START OBJECT</button><button id='geometry-finish' type='button'>FINISH OBJECT</button><button id='geometry-clear' type='button'>CLEAR</button><span id='geometry-status'>SELECT TEMPLATE</span></div>",
            "<div class='status' id='map-status'>MAPTILER HYBRID V4 · INITIALIZING</div>",
            "<section class='notam-boundaries' id='notam-boundaries' aria-label='NOTAM vertical boundaries'><h3>NOTAM · ВЕРТИКАЛЬНЫЕ ГРАНИЦЫ</h3><div id='notam-boundary-list'></div></section>",
            "<div class='controls'>",
            "<select id='style' aria-label='Map style'><option value='hybrid-v4' selected>HYBRID</option><option value='satellite-v4'>SATELLITE</option><option value='streets-v4'>STREETS</option></select>",
            "<button id='terrain' type='button'>3D TERRAIN: OFF</button>",
            "<button id='labels' class='active' type='button'>LABELS: ON</button>",
            "<button id='perf-snapshot' type='button'>PERF SNAPSHOT</button>",
            "</div>",
            "<div class='diag-panel' id='diag-panel'><div style='display:flex;gap:6px;align-items:center;color:#d5e7f7;font:12px monospace'><span>PERFORMANCE SNAPSHOT · API KEY REDACTED</span><button id='diag-refresh' type='button'>REFRESH</button><button id='diag-select' type='button'>SELECT ALL</button><button id='diag-close' type='button'>CLOSE</button></div><textarea id='diag-output' readonly spellcheck='false' aria-label='Performance diagnostics JSON'></textarea></div>",
            "<script src='qrc:///qtwebchannel/qwebchannel.js'></script>",
            "<script type='module'>",
            "import * as maplibregl from 'https://unpkg.com/maplibre-gl@6.14.0/dist/maplibre-gl.mjs';",
            "const apiKey=" + keyLiteral + ";",
            "const statusEl=document.getElementById('map-status');",
            "let geometryBridge=null;let selectedTemplates=[];let activeTemplate='';let capturing=false;let settingOrigin=false;let origin=null;let draft=[];let objects=[];let geometryVersion=1;",
            "const geometryTypes={'MT-01':'POLYGON_AOI','MT-02':'POLYGON_AOI','MT-03':'INSPECTION_POINTS','MT-04':'POLYGON_AOI','MT-05':'CORRIDOR','MT-06':'SEARCH_AREA','MT-07':'SEARCH_AREA','MT-08':'POLYGON_AOI','MT-09':'POLYGON_AOI','MT-10':'DELIVERY_ROUTE','MT-11':'OBSERVATION_POINTS','MT-12':'POLYGON_AOI','MT-13':'OBSERVATION_POINTS'};",
            "const geometryControls=document.getElementById('geometry-controls');const geometrySelect=document.getElementById('geometry-template');const geometryStatus=document.getElementById('geometry-status');",
            "function publishGeometry(complete){if(geometryBridge)geometryBridge.publishGeometry(JSON.stringify({origin:origin,objects:objects}),complete);}",
            "function syncGeometryTemplates(ids){selectedTemplates=Array.isArray(ids)?ids.filter(id=>geometryTypes[id]):[];geometrySelect.replaceChildren();selectedTemplates.forEach(id=>{const option=document.createElement('option');option.value=id;option.textContent=id+' · '+geometryTypes[id];geometrySelect.appendChild(option);});activeTemplate=selectedTemplates.includes(activeTemplate)?activeTemplate:(selectedTemplates[0]||'');geometrySelect.value=activeTemplate;geometryControls.style.display=selectedTemplates.length?'flex':'none';publishGeometry(false);}",
            "if(window.qt&&qt.webChannelTransport){new QWebChannel(qt.webChannelTransport,channel=>{geometryBridge=channel.objects.manualGeometryBridge;publishGeometry(false);});}",
            "if(!apiKey){statusEl.textContent='BLUESKY_MAPTILER_API_KEY NOT SET';}",
            "else{",
            "",
            "const map=new maplibregl.Map({container:'map',style:'https://api.maptiler.com/maps/hybrid-v4/style.json?key='+encodeURIComponent(apiKey),center:[31.95990,49.85842],zoom:6.7,attributionControl:true,fadeDuration:0,refreshExpiredTiles:false,cancelPendingTileRequestsWhileZooming:true,crossSourceCollisions:false,maxPitch:60,maxTileCacheZoomLevels:6,canvasContextAttributes:{antialias:false,preserveDrawingBuffer:false}});",
            "function refreshGeometryLayer(){if(!map.isStyleLoaded())return;const features=[];objects.forEach(o=>{const coords=o.coordinates.map(p=>[Number(p[0]),Number(p[1])]);if(['POLYGON_AOI','SEARCH_AREA'].includes(o.geometryType)&&coords.length>=3){const ring=coords.slice();if(ring[0][0]!==ring[ring.length-1][0]||ring[0][1]!==ring[ring.length-1][1])ring.push(ring[0]);features.push({type:'Feature',properties:{kind:'area'},geometry:{type:'Polygon',coordinates:[ring]}});}else if(['CORRIDOR','DELIVERY_ROUTE'].includes(o.geometryType)&&coords.length>=2){features.push({type:'Feature',properties:{kind:'line'},geometry:{type:'LineString',coordinates:coords}});}coords.forEach(p=>features.push({type:'Feature',properties:{kind:'point'},geometry:{type:'Point',coordinates:p}}));});draft.forEach(p=>features.push({type:'Feature',properties:{kind:'draft'},geometry:{type:'Point',coordinates:[p[0],p[1]]}}));if(origin)features.push({type:'Feature',properties:{kind:'origin'},geometry:{type:'Point',coordinates:[origin.longitudeDeg,origin.latitudeDeg]}});const data={type:'FeatureCollection',features};if(!map.getSource('bluesky-manual-geometry'))map.addSource('bluesky-manual-geometry',{type:'geojson',data});else map.getSource('bluesky-manual-geometry').setData(data);if(!map.getLayer('bluesky-manual-area'))map.addLayer({id:'bluesky-manual-area',type:'fill',source:'bluesky-manual-geometry',filter:['==',['get','kind'],'area'],paint:{'fill-color':'#32ffff','fill-opacity':0.16}});if(!map.getLayer('bluesky-manual-line'))map.addLayer({id:'bluesky-manual-line',type:'line',source:'bluesky-manual-geometry',filter:['==',['get','kind'],'line'],paint:{'line-color':'#64ff00','line-width':3}});if(!map.getLayer('bluesky-manual-points'))map.addLayer({id:'bluesky-manual-points',type:'circle',source:'bluesky-manual-geometry',filter:['in',['get','kind'],['literal',['point','draft','origin']]],paint:{'circle-radius':['match',['get','kind'],'origin',9,5],'circle-color':['match',['get','kind'],'origin','#64ff00','#32ffff'],'circle-stroke-color':'#ffffff','circle-stroke-width':1.5}});}",
            "window.blueskySetManualTemplates=function(ids){const next=Array.isArray(ids)?ids.filter(id=>geometryTypes[id]):[];if(JSON.stringify(next)!==JSON.stringify(selectedTemplates)){capturing=false;draft=[];}selectedTemplates=next;objects=objects.filter(o=>o.templateIds.every(id=>selectedTemplates.includes(id)));syncGeometryTemplates(selectedTemplates);refreshGeometryLayer();};",
            "const sidePanelBounds={left:" + JSON.stringify(root.leftPanelWidth) + ",right:" + JSON.stringify(root.rightPanelWidth) + "};",
            "document.addEventListener('wheel',e=>{const rect=document.getElementById('map').getBoundingClientRect();const x=e.clientX-rect.left;if(x<sidePanelBounds.left||x>=rect.width-sidePanelBounds.right){e.preventDefault();e.stopImmediatePropagation();}}, {capture:true,passive:false});",
            "window.blueskySetPanelBounds=b=>{sidePanelBounds.left=Math.max(0,Number(b.left)||0);sidePanelBounds.right=Math.max(0,Number(b.right)||0);};",
            "let notamSnapshotId='';let notamData={type:'FeatureCollection',features:[]};let notamPopup=null;let notamHoverPopup=null;",
            "function safeText(v){return String(v===undefined||v===null||v===''?'UNKNOWN':v).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');}",
            "function setNotams(records,snapshotId){notamSnapshotId=String(snapshotId||'');const features=[];const all=Array.isArray(records)?records:[];const boundaries=document.getElementById('notam-boundaries');const boundaryList=document.getElementById('notam-boundary-list');boundaryList.innerHTML=all.length?all.map(r=>{const id=safeText(r&&r.id||'UNKNOWN');const lower=safeText(r&&r.lowerLimitRaw||'UNKNOWN');const upper=safeText(r&&r.upperLimitRaw||'UNKNOWN');const datum=safeText(r&&r.altitudeReference||'UNKNOWN');const source=safeText(r&&r.source||'UNKNOWN');const validity=safeText(r&&r.effectiveStart||'UNKNOWN')+' — '+safeText(r&&r.effectiveEnd||'UNKNOWN');const geometryAvailable=(()=>{try{const g=JSON.parse(r&&r.geometryJson||'');return !!(g&&g.type);}catch(e){return false;}})();return '<div class=\'notam-boundary\'><b>'+id+'</b><br>Нижняя граница: '+lower+'<br>Верхняя граница: '+upper+'<br><span class=\'datum\'>Система высот: '+datum+' · Источник: '+source+'</span><br>Срок: '+validity+(geometryAvailable?'':'<br><b>Геометрия недоступна — границы показаны по исходной записи</b>')+'</div>';}).join(''):'<div class=\'notam-boundary\'>Нет загруженных записей NOTAM. Это не подтверждает отсутствие ограничений.</div>';boundaries.classList.toggle('open',all.length>0);all.forEach(r=>{if(!r||!r.geometryJson)return;let g;try{g=JSON.parse(r.geometryJson);}catch(e){return;}if(!g||!g.type)return;const p={id:String(r.id||'UNKNOWN'),number:String(r.number||''),source:String(r.source||'UNKNOWN'),validFrom:String(r.effectiveStart||'UNKNOWN'),validTo:String(r.effectiveEnd||'UNKNOWN'),lower:String(r.lowerLimitRaw||'UNKNOWN'),upper:String(r.upperLimitRaw||'UNKNOWN'),datum:String(r.altitudeReference||'UNKNOWN'),text:String(r.originalText||r.text||'Full source text unavailable'),snapshot:notamSnapshotId};features.push({type:'Feature',geometry:g,properties:p});});notamData={type:'FeatureCollection',features:features};if(map.isStyleLoaded())applyNotams();}",
            "function applyNotams(){if(!map.isStyleLoaded())return;if(!map.getSource('bluesky-notam-snapshot'))map.addSource('bluesky-notam-snapshot',{type:'geojson',data:notamData});else map.getSource('bluesky-notam-snapshot').setData(notamData);const layers=[{id:'bluesky-notam-fill',type:'fill',filter:['in',['geometry-type'],['literal',['Polygon','MultiPolygon']]],paint:{'fill-color':'#FFB020','fill-opacity':0.16}},{id:'bluesky-notam-line',type:'line',paint:{'line-color':'#FFB020','line-width':2.2,'line-opacity':0.95}},{id:'bluesky-notam-point',type:'circle',filter:['in',['geometry-type'],['literal',['Point','MultiPoint']]],paint:{'circle-radius':6,'circle-color':'#FFB020','circle-stroke-color':'#FFFFFF','circle-stroke-width':1.5}}];layers.forEach(l=>{if(!map.getLayer(l.id)){const layer={id:l.id,type:l.type,source:'bluesky-notam-snapshot',paint:l.paint};if(l.filter)layer.filter=l.filter;map.addLayer(layer);}});}",
            "function notamPopupHtml(p){return '<div style=\"max-width:340px;color:#102030;font:12px Arial,sans-serif\"><b>NOTAM '+safeText(p.id)+'</b><br>Источник: '+safeText(p.source)+'<br>Срок: '+safeText(p.validFrom)+' — '+safeText(p.validTo)+'<br>Нижняя граница: '+safeText(p.lower)+'<br>Верхняя граница: '+safeText(p.upper)+'<br>Вертикальная система: '+safeText(p.datum)+'<br>Снимок: '+safeText(p.snapshot)+'<hr><div style=\"white-space:pre-wrap;max-height:220px;overflow:auto\">'+safeText(p.text)+'</div></div>';}",
            "function notamHoverHtml(p){return '<div style=\"max-width:260px;color:#102030;font:12px Arial,sans-serif\"><b>NOTAM '+safeText(p.id)+'</b><br>'+safeText(p.lower)+' — '+safeText(p.upper)+' ('+safeText(p.datum)+')<br>Срок: '+safeText(p.validFrom)+' — '+safeText(p.validTo)+'<br><span style=\"color:#42566a\">Нажмите для полного текста</span></div>';}",
            "map.on('click',['bluesky-notam-fill','bluesky-notam-line','bluesky-notam-point'],e=>{if(!e.features||!e.features.length)return;const p=e.features[0].properties||{};if(notamPopup)notamPopup.remove();notamPopup=new maplibregl.Popup({maxWidth:'360px'}).setLngLat(e.lngLat).setHTML(notamPopupHtml(p)).addTo(map);});",
            "['bluesky-notam-fill','bluesky-notam-line','bluesky-notam-point'].forEach(id=>{map.on('mouseenter',id,e=>{map.getCanvas().style.cursor='pointer';if(notamHoverPopup)notamHoverPopup.remove();const p=e.features&&e.features.length?e.features[0].properties||{}:{};notamHoverPopup=new maplibregl.Popup({closeButton:false,closeOnClick:false,maxWidth:'280px',offset:8}).setLngLat(e.lngLat).setHTML(notamHoverHtml(p)).addTo(map);});map.on('mousemove',id,e=>{if(notamHoverPopup&&e.lngLat)notamHoverPopup.setLngLat(e.lngLat);});map.on('mouseleave',id,()=>{map.getCanvas().style.cursor='';if(notamHoverPopup){notamHoverPopup.remove();notamHoverPopup=null;}});});",
            "window.blueskySetNotams=setNotams;",
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
            "map.on('click',e=>{if(settingOrigin){origin={longitudeDeg:Number(e.lngLat.lng),latitudeDeg:Number(e.lngLat.lat),coordinateReferenceSystem:'EPSG:4326',originKind:'PLANNED_TAKEOFF'};settingOrigin=false;publishGeometry(false);geometryStatus.textContent='TAKEOFF ORIGIN SET · '+origin.longitudeDeg.toFixed(6)+', '+origin.latitudeDeg.toFixed(6);refreshGeometryLayer();return;}if(!capturing||!activeTemplate)return;const type=geometryTypes[activeTemplate];const altitudeInput=document.getElementById('geometry-agl');const altitude=Number(altitudeInput.value);if(['CORRIDOR','DELIVERY_ROUTE'].includes(type)&&(!Number.isFinite(altitude)||altitude<20||altitude>80)){geometryStatus.textContent='TARGET AGL MUST BE 20–80 m';return;}draft.push(['CORRIDOR','DELIVERY_ROUTE'].includes(type)?[Number(e.lngLat.lng),Number(e.lngLat.lat),altitude,'AGL']:[Number(e.lngLat.lng),Number(e.lngLat.lat)]);geometryStatus.textContent=activeTemplate+' · '+draft.length+' POINT(S)'+(['CORRIDOR','DELIVERY_ROUTE'].includes(type)?' · AGL '+altitude+' m':'');refreshGeometryLayer();});",
            "document.getElementById('geometry-origin').addEventListener('click',()=>{capturing=false;draft=[];settingOrigin=true;geometryStatus.textContent='CLICK MAP TO SET PLANNED TAKEOFF ORIGIN';});geometrySelect.addEventListener('change',e=>{activeTemplate=e.target.value;});",
            "document.getElementById('geometry-start').addEventListener('click',()=>{if(!activeTemplate){geometryStatus.textContent='SELECT TEMPLATE';return;}capturing=true;draft=[];geometryStatus.textContent='CLICK MAP · '+activeTemplate+' · '+geometryTypes[activeTemplate];});",
            "document.getElementById('geometry-finish').addEventListener('click',()=>{if(!capturing||!activeTemplate)return;const type=geometryTypes[activeTemplate];const minimum=['POLYGON_AOI','SEARCH_AREA'].includes(type)?3:(['CORRIDOR','DELIVERY_ROUTE'].includes(type)?2:1);if(draft.length<minimum){geometryStatus.textContent='INSUFFICIENT POINTS · NEED '+minimum;return;}objects=objects.filter(o=>!o.templateIds.includes(activeTemplate));const finalCoordinates=draft.slice();if(['POLYGON_AOI','SEARCH_AREA'].includes(type)&&(finalCoordinates[0][0]!==finalCoordinates[finalCoordinates.length-1][0]||finalCoordinates[0][1]!==finalCoordinates[finalCoordinates.length-1][1]))finalCoordinates.push(finalCoordinates[0]);objects.push({geometryId:'GEO-'+activeTemplate+'-'+geometryVersion++,geometryType:type,coordinateReferenceSystem:'EPSG:4326',coordinates:finalCoordinates,templateIds:[activeTemplate],version:geometryVersion});draft=[];capturing=false;const covered=new Set(objects.flatMap(o=>o.templateIds));const complete=selectedTemplates.length>0&&selectedTemplates.every(id=>covered.has(id));publishGeometry(complete);geometryStatus.textContent=complete?'GEOMETRY COMPLETE':'OBJECT SAVED · CONTINUE WITH OTHER TEMPLATES';refreshGeometryLayer();});",
            "document.getElementById('geometry-clear').addEventListener('click',()=>{objects=[];draft=[];capturing=false;settingOrigin=false;origin=null;geometryVersion++;publishGeometry(false);refreshGeometryLayer();geometryStatus.textContent='GEOMETRY AND ORIGIN CLEARED';});",
            "map.on('load',()=>{statusEl.textContent='MAPTILER HYBRID V4 · READY';});",
            "map.on('style.load',()=>{const selectedStyle=document.getElementById('style').value;statusEl.textContent='MAPTILER '+selectedStyle.toUpperCase()+' · READY';});",
            "map.on('error',e=>{perfDiag.errors++;const err=e&&e.error?e.error:null;const item={atMs:Math.round(performance.now()),type:e&&e.type?String(e.type):'unknown',message:err&&err.message?String(err.message):'Map error without error.message',stack:err&&err.stack?String(err.stack).slice(0,4000):null,sourceId:e&&e.sourceId?String(e.sourceId):null,sourceType:e&&e.sourceType?String(e.sourceType):null,tileId:e&&e.tile&&e.tile.uid!==undefined?String(e.tile.uid):null};perfDiag.recentErrors.push(item);if(perfDiag.recentErrors.length>30)perfDiag.recentErrors.shift();console.warn('MapTiler map error:',item.message);});",
            "function enableTerrain(){if(!map.getSource('bluesky-terrain-dem'))map.addSource('bluesky-terrain-dem',{type:'raster-dem',url:'https://api.maptiler.com/tiles/terrain-rgb-v2/tiles.json?key='+encodeURIComponent(apiKey),tileSize:256});map.setTerrain({source:'bluesky-terrain-dem',exaggeration:1.0});}map.on('style.load',()=>{applyRoute();applyNotams();applyLabels(true);refreshGeometryLayer();if(terrainOn){try{enableTerrain();}catch(e){console.warn('Terrain unavailable:',e.message);}}});",
            "document.getElementById('style').addEventListener('change',e=>{const id=e.target.value;perfDiag.styleChanges++;perfDiag.lastStyleStart=performance.now();performance.mark('bluesky-map-style-change');statusEl.textContent='MAPTILER '+id.toUpperCase()+' · LOADING';map.setStyle('https://api.maptiler.com/maps/'+id+'/style.json?key='+encodeURIComponent(apiKey),{diff:false});});",
            "document.getElementById('terrain').addEventListener('click',()=>{terrainOn=!terrainOn;try{if(terrainOn){if(map.isStyleLoaded())enableTerrain();map.easeTo({pitch:55,duration:250});}else{map.setTerrain(null);map.easeTo({pitch:0,duration:350});}document.getElementById('terrain').textContent='3D TERRAIN: '+(terrainOn?'ON':'OFF');document.getElementById('terrain').classList.toggle('active',terrainOn);}catch(e){terrainOn=false;statusEl.textContent='3D TERRAIN UNAVAILABLE';console.warn('Terrain toggle failed:',e.message);}});",
            "document.getElementById('labels').addEventListener('click',()=>{labelsVisible=!labelsVisible;applyLabels(false);document.getElementById('labels').textContent='LABELS: '+(labelsVisible?'ON':'OFF');document.getElementById('labels').classList.toggle('active',labelsVisible);});",
            "window.blueskySetRoute(" + JSON.stringify(root.routeCoordinates || []) + ");",
            "window.blueskySetNotams(" + JSON.stringify(root.notamRecords || []) + "," + JSON.stringify(root.notamSnapshotId || "") + ");",
            "window.blueskySetManualTemplates(" + JSON.stringify(root.manualTemplateIds || []) + ");",
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

    function syncManualTemplates() {
        if (htmlLoaded && !webMap.loading) {
            var payload = JSON.stringify(root.manualTemplateIds || [])
            webMap.runJavaScript("if(window.blueskySetManualTemplates)window.blueskySetManualTemplates(" + payload + ");")
        }
    }

    function syncNotamRecords() {
        if (htmlLoaded && !webMap.loading) {
            var payload = JSON.stringify(root.notamRecords || [])
            var snapshot = JSON.stringify(root.notamSnapshotId || "")
            webMap.runJavaScript("if(window.blueskySetNotams)window.blueskySetNotams(" + payload + "," + snapshot + ");")
        }
    }

    function syncPanelBounds() {
        if (htmlLoaded && !webMap.loading) {
            var payload = JSON.stringify({ left: root.leftPanelWidth, right: root.rightPanelWidth })
            webMap.runJavaScript("if(window.blueskySetPanelBounds)window.blueskySetPanelBounds(" + payload + ");")
        }
    }

    // Diagnostic step 2: reconnect the Qt WebChannel transport without registering
    // any QObject. This separates transport hookup from registered-object lookup.
    WebChannel {
        id: mapWebChannel
    }

    WebEngineView {
        id: webMap
        webChannel: root.apiKey.length > 0 ? mapWebChannel : null
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
                root.syncManualTemplates()
                root.syncNotamRecords()
                root.syncPanelBounds()
            } else if (request.status === WebEngineView.LoadFailedStatus) {
                console.warn("MapTiler SDK load failed:", request.errorString)
            }
        }
        Component.onCompleted: {
            if (root.apiKey.length > 0)
                loadHtml(root.buildHtml(), "https://unpkg.com/")
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
        function onManualTemplateIdsChanged() { root.syncManualTemplates() }
        function onNotamRecordsChanged() { root.syncNotamRecords() }
        function onNotamSnapshotIdChanged() { root.syncNotamRecords() }
        function onLeftPanelWidthChanged() { root.syncPanelBounds() }
        function onRightPanelWidthChanged() { root.syncPanelBounds() }
    }
}
