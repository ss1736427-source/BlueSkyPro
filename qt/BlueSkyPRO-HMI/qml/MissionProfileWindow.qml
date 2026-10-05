import QtQuick
import QtCore

Item {
    id: root
    property string missionId: ""
    property var uavModel: []
    // Assignment result supplied by mission planning; the HMI does not infer task allocation.
    property var missionAssignments: []
    property int selectedUavIndex: -1
    signal uavSelectionRequested(int index)
    property var routeDataByUav: ({})
    property var mandatoryPointsByUav: ({})
    property string loadedUavId: ""
    property string missionSummary: ""
    property string missionReviewState: ""
    property var missionTemplateIndices: []
    signal closeRequested()
    signal applyRequested()

    readonly property color bg: "#07111E"
    readonly property color panel: "#0B1B2B"
    readonly property color line: "#174056"
    readonly property color cyan: "#00DDF2"
    readonly property color textColor: "#DCE8F2"
    readonly property color muted: "#91A8BA"
    readonly property color switchGreen: "#39D353"
    property bool parameterPanelOpen: false
    property int columnLayoutRevision: 0
    property real visibleColumnWeightValue: 1.0
    property real tableSplitRatio: 0.47
    // Live telemetry inputs; connect these to the flight-data source when available.
    property bool liveFlightActive: false
    // Design-time marker keeps the aircraft visible in Qt Design Studio.
    // A connected telemetry feed overrides this preview position.
    property bool showAircraftPreview: true
    property real previewDistanceKm: 46.7
    property real previewAltitudeM: 180
    property real liveDistanceKm: 0
    property real liveElapsedSeconds: 0
    property real liveAltitudeM: 0
    // Editable profile constraints. Progress is normalized along the route.
    property var mandatoryPoints: []
    readonly property real plannedDistanceKm: 78.4
    readonly property real plannedDurationSeconds: 78 * 60
    property var parameterVisibility: ({
        number: true, type: true, point: true, course: true, distance: true, altitude: true,
        airspeed: true, groundspeed: true, time: true, deltaHeight: true, energy: true, note: true
    })
    property var columns: [
        { label: "#", w: 0.035, key: "number" },
        { label: "ТИП", w: 0.075, key: "type" },
        { label: "ТОЧКА / ШИРОТА, ДОЛГОТА", w: 0.17, key: "point" },
        { label: "КУРС\n°", w: 0.065, key: "course" },
        { label: "ДИСТАНЦИЯ\nкм", w: 0.075, key: "distance" },
        { label: "ВЫСОТА\nм", w: 0.075, key: "altitude" },
        { label: "V_ВОЗД\nм/с", w: 0.075, key: "airspeed" },
        { label: "V_ПУТ\nм/с", w: 0.075, key: "groundspeed" },
        { label: "ВРЕМЯ\nмин", w: 0.075, key: "time" },
        { label: "Δh\nм", w: 0.06, key: "deltaHeight" },
        { label: "ЭНЕРГИЯ\n%", w: 0.07, key: "energy" },
        { label: "ПРИМЕЧАНИЕ", w: 0.15, key: "note" }
    ]
    property var columnOrder: ["number", "type", "point", "course", "distance", "altitude",
                               "airspeed", "groundspeed", "time", "deltaHeight", "energy", "note"]
    function orderedColumns() {
        var result = []
        for (var i = 0; i < root.columnOrder.length; ++i)
            for (var j = 0; j < root.columns.length; ++j)
                if (root.columns[j].key === root.columnOrder[i]) result.push(root.columns[j])
        return result
    }
    function recalculateColumnLayout() {
        var all = root.orderedColumns()
        var total = 0
        for (var i = 0; i < all.length; ++i) {
            if (root.parameterVisibility[all[i].key] !== false)
                total += all[i].w
        }
        root.visibleColumnWeightValue = Math.max(0.001, total)
        root.columnLayoutRevision += 1
    }

    function columnWidth(column) {
        if (root.parameterVisibility[column.key] === false) return 0
        var revision = root.columnLayoutRevision
        return tablePanel.width * column.w / root.visibleColumnWeightValue
    }

    function columnAtX(x) {
        var all = root.orderedColumns()
        var cursor = 0
        var candidates = []
        for (var i = 0; i < all.length; ++i) {
            if (root.parameterVisibility[all[i].key] === false) continue
            var width = root.columnWidth(all[i])
            candidates.push({ key: all[i].key, center: cursor + width / 2 })
            cursor += width
        }
        if (candidates.length === 0) return ""
        for (var j = 0; j < candidates.length; ++j)
            if (x < candidates[j].center) return candidates[j].key
        return candidates[candidates.length - 1].key
    }
    function moveColumn(fromKey, toKey) {
        if (!fromKey || !toKey || fromKey === toKey) return
        var next = root.columnOrder.slice(0)
        var from = next.indexOf(fromKey), to = next.indexOf(toKey)
        if (from < 0 || to < 0) return
        next.splice(from, 1)
        next.splice(to, 0, fromKey)
        root.columnOrder = next
        profileSettings.columnOrderJson = JSON.stringify(next)
    }
    property var tableRows: []

    // The profile's horizontal progress is the shared ordering source for
    // both marker labels and table rows. routeIndex may be stale after dragging.
    function mandatoryRoutePosition(point) {
        return Number(point.progress)
    }

    function rebuildTableRows() {
        var rows = []
        var mandatory = root.mandatoryPoints.slice(0)
        var denominator = Math.max(1, routeModel.count - 1)

        // Route-bound mandatory points retain the identity and sequence slot
        // of their route waypoint. Standalone mandatory points are inserted
        // into the same ordered list below.
        for (var i = 0; i < routeModel.count; i++) {
            var source = routeModel.get(i)
            var progress = i / denominator
            var bound = null
            for (var m = 0; m < mandatory.length; m++) {
                if (Number(mandatory[m].routeIndex) === i && Number(mandatory[m].routeIndex) >= 0) {
                    bound = mandatory[m]
                    break
                }
            }
            rows.push({
                progress: bound ? Number(bound.progress) : progress,
                order: i,
                pointType: bound ? "Обязательная" : source.pointType,
                pointName: source.pointName,
                coordinates: source.coordinates,
                course: source.course,
                distance: bound ? distanceAtProgress(bound.progress).toFixed(1) : source.distance,
                altitude: bound ? Math.round(Number(bound.altitude)) : source.altitude,
                airspeed: source.airspeed,
                groundspeed: source.groundspeed,
                time: source.time,
                deltaHeight: source.deltaHeight,
                energy: source.energy,
                note: bound ? "Обязательная точка" : source.note,
                isMandatory: bound !== null,
                mandatoryId: bound ? bound.id : "",
                routeIndex: i
            })
        }

        // Unbound mandatory points become route nodes between the surrounding
        // waypoints and receive the same sequential number as every other node.
        for (var j = 0; j < mandatory.length; j++) {
            var point = mandatory[j]
            if (Number(point.routeIndex) >= 0) continue
            var mp = Math.max(0, Math.min(1, Number(point.progress)))
            var segment = mp * denominator
            var lowerIndex = Math.min(routeModel.count - 1, Math.floor(segment))
            var upperIndex = Math.min(routeModel.count - 1, lowerIndex + 1)
            var fraction = segment - lowerIndex
            var lower = routeModel.get(lowerIndex)
            var upper = routeModel.get(upperIndex)
            var lowerCoords = String(lower.coordinates).split(",")
            var upperCoords = String(upper.coordinates).split(",")
            var latA = Number(lowerCoords[0]), lonA = Number(lowerCoords[1])
            var latB = Number(upperCoords[0]), lonB = Number(upperCoords[1])
            var coords = isFinite(latA) && isFinite(lonA) && isFinite(latB) && isFinite(lonB)
                         ? (latA + (latB-latA)*fraction).toFixed(4) + ", " + (lonA + (lonB-lonA)*fraction).toFixed(4)
                         : "—"
            rows.push({
                progress: mp,
                order: segment + 0.5,
                pointType: "Обязательная",
                pointName: "Обязательная точка",
                coordinates: coords,
                course: "—",
                distance: distanceAtProgress(mp).toFixed(1),
                altitude: Math.round(Number(point.altitude)),
                airspeed: "—",
                groundspeed: "—",
                time: "—",
                deltaHeight: "—",
                energy: "—",
                note: "Требует пересчёта",
                isMandatory: true,
                mandatoryId: point.id,
                routeIndex: -1
            })
        }

        rows.sort(function(a, b) {
            if (a.progress !== b.progress) return a.progress - b.progress
            return a.order - b.order
        })
        root.tableRows = rows
    }

    function distanceAtProgress(progress) {
        if (routeModel.count < 2) return 0
        var p = Math.max(0, Math.min(1, Number(progress)))
        var scaled = p * (routeModel.count - 1)
        var index = Math.min(routeModel.count - 2, Math.floor(scaled))
        var fraction = scaled - index
        var d1 = Number(routeModel.get(index).distance)
        var d2 = Number(routeModel.get(index + 1).distance)
        if (!isFinite(d1)) d1 = 0
        if (!isFinite(d2)) d2 = d1
        return Math.max(0, d1 + (d2 - d1) * fraction)
    }

    function columnValue(rowIndex, key) {
        var row = root.tableRows[rowIndex]
        if (!row) return ""
        var values = { number: String(rowIndex + 1), type: row.pointType,
            point: row.pointName + "\n" + row.coordinates, course: row.course,
            distance: row.distance, altitude: row.altitude, airspeed: row.airspeed,
            groundspeed: row.groundspeed, time: row.time, deltaHeight: row.deltaHeight,
            energy: row.energy, note: row.note }
        return values[key] === undefined ? "" : values[key]
    }

    function setTableAltitude(rowIndex, value) {
        var row = root.tableRows[rowIndex], altitude = Number(value)
        if (!row || !isFinite(altitude) || altitude < 0 || altitude > 5000) return
        if (row.isMandatory) {
            var next = root.mandatoryPoints.slice(0)
            for (var i = 0; i < next.length; i++) {
                if (next[i].id === row.mandatoryId) {
                    next[i] = { id: next[i].id, progress: next[i].progress, altitude: altitude,
                        routeIndex: next[i].routeIndex === undefined ? -1 : next[i].routeIndex }
                    var boundIndex = Number(next[i].routeIndex)
                    if (boundIndex >= 0 && boundIndex < routeModel.count)
                        routeModel.setProperty(boundIndex, "altitude", String(altitude))
                    root.mandatoryPoints = next
                    root.rebuildTableRows()
                    profileCanvas.requestPaint()
                    return
                }
            }
        } else if (row.routeIndex >= 0) routeModel.setProperty(row.routeIndex, "altitude", String(altitude))
    }
    function parameterKey(i) {
        return ["number", "type", "point", "course", "distance", "altitude",
                "airspeed", "groundspeed", "time", "deltaHeight", "energy", "note"][i]
    }
    function toggleParameter(key) {
        var next = Object.assign({}, root.parameterVisibility)
        next[key] = !next[key]
        root.parameterVisibility = next
        root.recalculateColumnLayout()
    }

    Settings {
        id: profileSettings
        category: "BlueSkyPRO/MissionProfile"
        property string columnOrderJson: ""
        property real tableSplitRatio: 0.47
        property string routeDataByUavJson: "{}"
        property string mandatoryPointsByUavJson: "{}"
    }

    function assignmentForUav(uavId) {
        for (var i = 0; i < root.missionAssignments.length; ++i)
            if (String(root.missionAssignments[i].uavId) === String(uavId))
                return root.missionAssignments[i]
        return null
    }

    function selectedAircraftId() {
        return root.selectedUavIndex >= 0 && root.selectedUavIndex < root.uavModel.length
               ? String(root.uavModel[root.selectedUavIndex].id) : "DEFAULT"
    }

    function compactMissionFunction() {
        var summary = String(root.missionSummary).toLowerCase()
        if (summary.indexOf("3d") >= 0 && summary.indexOf("карт") >= 0) return "3D-картография"
        if (summary.indexOf("карт") >= 0 || summary.indexOf("съём") >= 0 || summary.indexOf("съем") >= 0) return "Картография"
        if (summary.indexOf("инспек") >= 0) return "Инспекция"
        if (summary.indexOf("монитор") >= 0) return "Мониторинг"
        if (summary.indexOf("лидар") >= 0 || summary.indexOf("lidar") >= 0) return "Лидар"
        return root.missionSummary
    }

    function saveCurrentAircraftData() {
        var id = root.loadedUavId.length > 0 ? root.loadedUavId : root.selectedAircraftId()
        var routes = Object.assign({}, root.routeDataByUav)
        var points = Object.assign({}, root.mandatoryPointsByUav)
        var snapshot = []
        for (var i = 0; i < routeModel.count; ++i) {
            var item = routeModel.get(i), copy = {}
            for (var key in item) copy[key] = item[key]
            snapshot.push(copy)
        }
        routes[id] = snapshot
        points[id] = root.mandatoryPoints
        root.routeDataByUav = routes
        root.mandatoryPointsByUav = points
        profileSettings.routeDataByUavJson = JSON.stringify(routes)
        profileSettings.mandatoryPointsByUavJson = JSON.stringify(points)
    }

    function loadAircraftData(index) {
        var id = index >= 0 && index < root.uavModel.length
                 ? String(root.uavModel[index].id) : "DEFAULT"
        root.loadedUavId = id
        var routes = root.routeDataByUav[id]
        if (Array.isArray(routes) && routes.length > 0) {
            routeModel.clear()
            for (var i = 0; i < routes.length; ++i) routeModel.append(routes[i])
        }
        root.mandatoryPoints = Array.isArray(root.mandatoryPointsByUav[id])
                               ? root.mandatoryPointsByUav[id] : []
        root.rebuildTableRows()
        profileCanvas.requestPaint()
    }

    function selectAircraft(index) {
        if (index < 0 || index >= root.uavModel.length || index === root.selectedUavIndex) return
        root.saveCurrentAircraftData()
        root.uavSelectionRequested(index)
    }

    onSelectedUavIndexChanged: root.loadAircraftData(root.selectedUavIndex)

    Component.onCompleted: {
        root.recalculateColumnLayout()
        root.tableSplitRatio = Math.max(0.25, Math.min(0.75, profileSettings.tableSplitRatio))
        try { root.routeDataByUav = JSON.parse(profileSettings.routeDataByUavJson) || ({}) }
        catch (e) { root.routeDataByUav = ({}) }
        try { root.mandatoryPointsByUav = JSON.parse(profileSettings.mandatoryPointsByUavJson) || ({}) }
        catch (e) { root.mandatoryPointsByUav = ({}) }
        var initialRoute = []
        for (var ri = 0; ri < routeModel.count; ++ri) {
            var source = routeModel.get(ri), item = {}
            for (var key in source) item[key] = source[key]
            initialRoute.push(item)
        }
        var initialId = root.selectedAircraftId()
        var routes = Object.assign({}, root.routeDataByUav)
        var points = Object.assign({}, root.mandatoryPointsByUav)
        for (var ui = 0; ui < root.uavModel.length; ++ui) {
            var uid = String(root.uavModel[ui].id)
            if (!Array.isArray(routes[uid])) routes[uid] = initialRoute
            if (!Array.isArray(points[uid])) points[uid] = []
        }
        if (!Array.isArray(routes[initialId])) routes[initialId] = initialRoute
        if (!Array.isArray(points[initialId])) points[initialId] = []
        root.routeDataByUav = routes
        root.mandatoryPointsByUav = points
        root.loadAircraftData(root.selectedUavIndex)
        if (profileSettings.columnOrderJson.length > 0) {
            try {
                var saved = JSON.parse(profileSettings.columnOrderJson)
                if (Array.isArray(saved) && saved.length === root.columns.length)
                    root.columnOrder = saved
            } catch (e) { }
        }
    }

    onTableSplitRatioChanged: profileSettings.tableSplitRatio = tableSplitRatio
    onMandatoryPointsChanged: {
        root.mandatoryPointsByUav[root.selectedAircraftId()] = mandatoryPoints
        profileSettings.mandatoryPointsByUavJson = JSON.stringify(root.mandatoryPointsByUav)
        rebuildTableRows()
        profileCanvas.requestPaint()
    }

    anchors.fill: parent
    z: 80
    visible: root.visible

    Rectangle {
        anchors.fill: parent
        color: "#B8000710"
    }

    Rectangle {
        id: dialog
        width: Math.min(1540, parent.width - 18)
        height: Math.min(900, parent.height - 16)
        anchors.centerIn: parent
        color: root.bg
        border.color: root.cyan
        border.width: 1
        radius: 5

        Column {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 7

            Rectangle {
                id: titleBar
                width: parent.width
                height: 34
                color: root.panel
                Text {
                    id: missionTitleText
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    y: (parent.height - height) / 2
                    text: "МИССИЯ"
                    color: root.cyan
                    font.family: "B612"
                    font.pixelSize: 15
                    font.bold: true
                }
                Text {
                    id: missionIdText
                    anchors.left: missionTitleText.right
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.missionId
                    color: root.cyan
                    font.family: "B612 Mono"
                    font.pixelSize: 12
                }
                Row {
                    id: uavTabs
                    anchors.left: missionIdText.right
                    anchors.leftMargin: 14
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.min(implicitWidth, Math.max(0, collapsedToolsToggle.x - x - 12))
                    height: 34
                    spacing: 6
                    clip: true

                    Repeater {
                        model: root.uavModel
                        delegate: Rectangle {
                            required property int index
                            required property var modelData
                            width: Math.max(120, Math.min(250, tabLabel.implicitWidth + 16))
                            height: uavTabs.height
                            radius: 3
                            color: index === root.selectedUavIndex ? "#102B3A" : "#091725"
                            border.color: index === root.selectedUavIndex ? root.cyan : root.line
                            border.width: index === root.selectedUavIndex ? 1.5 : 1

                            Text {
                                id: tabLabel
                                anchors.fill: parent
                                anchors.leftMargin: 5
                                anchors.rightMargin: 5
                                property var assignment: root.assignmentForUav(modelData.id)
                                text: (index + 1) + " · " + (assignment ? assignment.task : "Задача не назначена")
                                      + "\n" + (assignment ? assignment.sector : "Сектор не назначен")
                                      + " · " + modelData.id
                                color: index === root.selectedUavIndex ? root.cyan : root.textColor
                                font.family: "B612"
                                font.pixelSize: 10
                                font.bold: index === root.selectedUavIndex
                                fontSizeMode: Text.Fit
                                minimumPixelSize: 7
                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.selectAircraft(index)
                            }
                        }
                    }
                }

                // When the parameter panel is collapsed, keep its tools button
                // visible in the title bar, immediately left of the close button.
                Text {
                    id: collapsedToolsToggle
                    visible: !root.parameterPanelOpen
                    anchors.right: parent.right
                    anchors.rightMargin: 42
                    anchors.verticalCenter: parent.verticalCenter
                    text: "☰"
                    color: root.cyan
                    font.pixelSize: 22
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -8
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.parameterPanelOpen = true
                    }
                }
                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: "×"
                    color: root.cyan
                    font.pixelSize: 22
                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -7
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.saveCurrentAircraftData()
                            root.closeRequested()
                        }
                    }
                }
            }

            Row {
                id: contentRow
                width: parent.width
                height: parent.height - titleBar.height - 7
                spacing: 8

                Column {
                    id: mainColumn
                    width: root.parameterPanelOpen
                           ? parent.width - parameterPanel.width - parent.spacing
                           : parent.width
                    height: parent.height
                    Behavior on width { NumberAnimation { duration: 160 } }
                    spacing: 0

                    Rectangle {
                        id: tablePanel
                        width: parent.width
                        height: Math.round((mainColumn.height - splitHandle.height) * root.tableSplitRatio)
                        color: root.bg
                        border.color: root.line
                        radius: 3

                        Column {
                            anchors.fill: parent
                            spacing: 0

                            Row {
                                id: tableHeader
                                width: parent.width
                                height: 40
                                spacing: 0
                                Repeater {
                                    model: root.orderedColumns()
                                    delegate: Rectangle {
                                        id: headerCell
                                        width: root.columnWidth(modelData)
                                        height: tableHeader.height
                                        visible: root.parameterVisibility[modelData.key] !== false
                                        color: headerDragArea.pressed ? "#12394A" : "#0B1B2B"
                                        border.color: root.line
                                        Text {
                                            anchors.fill: parent
                                            anchors.margins: 3
                                            text: modelData.label
                                            color: root.textColor
                                            font.family: "B612"
                                            font.pixelSize: 10
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            wrapMode: Text.Wrap
                                        }
                                        MouseArea {
                                            id: headerDragArea
                                            anchors.fill: parent
                                            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                                            drag.target: null
                                            property real pressX: 0
                                            property real pressY: 0
                                            property bool didDrag: false

                                            onPressed: {
                                                pressX = mouse.x
                                                pressY = mouse.y
                                                didDrag = false
                                            }

                                            onPositionChanged: {
                                                if (pressed &&
                                                    (Math.abs(mouse.x - pressX) > Qt.styleHints.startDragDistance ||
                                                     Math.abs(mouse.y - pressY) > Qt.styleHints.startDragDistance)) {
                                                    didDrag = true
                                                }
                                            }

                                            onReleased: {
                                                if (!didDrag) return
                                                var p = headerDragArea.mapToItem(tableHeader, mouse.x, mouse.y)
                                                root.moveColumn(modelData.key, root.columnAtX(p.x))
                                            }
                                        }
                                    }
                                }
                            }

                            ListView {
                                id: routeTable
                                width: parent.width
                                height: parent.height - tableHeader.height
                                clip: true
                                model: root.tableRows
                                delegate: Row {
                                    id: routeRowDelegate
                                    width: routeTable.width
                                    height: Math.max(37, Math.min(48, routeTable.height / 6))
                                    spacing: 0
                                    property int rowIndex: index
                                    Repeater {
                                        model: root.orderedColumns()
                                        delegate: Rectangle {
                                            width: root.columnWidth(modelData)
                                            height: parent.height
                                            visible: root.parameterVisibility[modelData.key] !== false
                                            color: routeTable.currentIndex === routeRowDelegate.rowIndex ? "#102B3A" : (rowIndex % 2 ? "#091725" : "#0C1D2C")
                                            border.color: root.line
                                            Text {
                                                visible: modelData.key !== "altitude"
                                                anchors.fill: parent
                                                anchors.margins: 4
                                                text: root.columnValue(routeRowDelegate.rowIndex, modelData.key)
                                                color: modelData.key === "energy"
                                                       ? (Number(root.columnValue(routeRowDelegate.rowIndex, modelData.key)) > 70 ? "#64FF00" : "#FFD339")
                                                       : (modelData.key === "type" && root.tableRows[routeRowDelegate.rowIndex].isMandatory ? "#155BFF" : root.textColor)
                                                font.family: "B612"
                                                font.pixelSize: 10
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                                wrapMode: Text.Wrap
                                                elide: Text.ElideRight
                                            }
                                            // Altitude cells are editable in the design preview.
                                            Rectangle {
                                                visible: modelData.key === "altitude"
                                                anchors.fill: parent
                                                anchors.margins: 5
                                                color: "transparent"
                                                border.color: "#31566A"
                                                radius: 2
                                                TextInput {
                                                    anchors.fill: parent
                                                    anchors.margins: 2
                                                    text: root.columnValue(routeRowDelegate.rowIndex, "altitude")
                                                    color: root.textColor
                                                    font.family: "B612 Mono"
                                                    font.pixelSize: 11
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                    selectByMouse: true
                                                    validator: IntValidator { bottom: 0; top: 5000 }
                                                    onEditingFinished: root.setTableAltitude(routeRowDelegate.rowIndex, text)
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Transparent vertical splitter: drag up/down to resize the
                    // table viewport and the flight-profile plot proportionally.
                    Item {
                        id: splitHandle
                        width: parent.width
                        height: 1
                        z: 2

                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width
                            height: 1
                            color: splitMouse.containsMouse ? root.cyan : root.line
                            opacity: splitMouse.containsMouse ? 0.95 : 0.65
                        }

                        MouseArea {
                            id: splitMouse
                            anchors.fill: parent
                            // Keep a practical drag target while the visual gap is 1 px.
                            anchors.margins: -5
                            hoverEnabled: true
                            cursorShape: Qt.SplitVCursor
                            property real lastY: 0

                            onPressed: lastY = mouse.y
                            onPositionChanged: {
                                if (!pressed) return
                                var delta = mouse.y - lastY
                                lastY = mouse.y
                                var available = mainColumn.height - splitHandle.height
                                var minTable = 150
                                var minProfile = 190
                                var nextHeight = tablePanel.height + delta
                                nextHeight = Math.max(minTable, Math.min(available - minProfile, nextHeight))
                                root.tableSplitRatio = nextHeight / available
                            }
                        }
                    }

                    Rectangle {
                        id: profilePanel
                        width: parent.width
                        height: mainColumn.height - tablePanel.height - splitHandle.height
                        color: root.bg
                        border.color: root.line
                        radius: 3

                        Column {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4
                            Row {
                                width: parent.width
                                height: 26
                                Text {
                                    text: "ПРОФИЛЬ ПОЛЁТА"
                                    color: root.cyan
                                    font.family: "B612"
                                    font.pixelSize: 15
                                    font.bold: true
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Item { width: 18; height: 1 }
                                Text {
                                    text: "━ Профиль маршрута     ▰ Рельеф местности     ┄ Ограничение высоты     ○ Точки маршрута     ➜ Ветер"
                                    color: root.muted
                                    font.family: "B612"
                                    font.pixelSize: 9
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            Canvas {
                                id: profileCanvas
                                width: parent.width
                                height: parent.height - 30
                                onWidthChanged: requestPaint()
                                onHeightChanged: requestPaint()
                                Connections {
                                    target: root
                                    // Redraw route-node labels whenever a mandatory point moves,
                                    // is added, or is removed. The Canvas computes numbering from
                                    // the current progress-sorted route nodes during onPaint.
                                    function onMandatoryPointsChanged() { profileCanvas.requestPaint() }
                                    function onLiveFlightActiveChanged() { profileCanvas.requestPaint() }
                                    function onLiveDistanceKmChanged() { profileCanvas.requestPaint() }
                                    function onLiveElapsedSecondsChanged() { profileCanvas.requestPaint() }
                                    function onLiveAltitudeMChanged() { profileCanvas.requestPaint() }
                                }
                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    var left = 42, right = width - 12, top = 18, bottom = height - 28
                                    var plotW = right - left, plotH = bottom - top
                                    ctx.strokeStyle = "#17384A"
                                    ctx.lineWidth = 1
                                    for (var gy = 0; gy <= 4; gy++) {
                                        var y = top + plotH * gy / 4
                                        ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(right, y); ctx.stroke()
                                    }
                                    for (var gx = 0; gx <= 8; gx++) {
                                        var x = left + plotW * gx / 8
                                        ctx.beginPath(); ctx.moveTo(x, top); ctx.lineTo(x, bottom); ctx.stroke()
                                    }
                                    // Terrain area
                                    var terrain = [130,70,105,145,82,150,90,165,120,75,135,155,95,205,280,230,160,205,130,90,120,195]
                                    ctx.beginPath(); ctx.moveTo(left, bottom)
                                    for (var i = 0; i < terrain.length; i++) {
                                        var tx = left + plotW * i / (terrain.length - 1)
                                        var ty = bottom - (terrain[i] / 400) * plotH
                                        ctx.lineTo(tx, ty)
                                    }
                                    ctx.lineTo(right, bottom); ctx.closePath()
                                    ctx.fillStyle = "#294B2D"; ctx.fill()
                                    ctx.strokeStyle = "#64A83B"; ctx.lineWidth = 1; ctx.stroke()
                                    // Altitude restriction
                                    ctx.setLineDash([8, 5]); ctx.strokeStyle = "#FF3548"; ctx.lineWidth = 2
                                    ctx.beginPath(); ctx.moveTo(left, top + plotH * 0.12); ctx.lineTo(right, top + plotH * 0.12); ctx.stroke()
                                    ctx.setLineDash([])
                                    // Route profile with mandatory points inserted into the same
                                    // continuous polyline, ordered by route progress.
                                    var alts = []
                                    for (var ai = 0; ai < routeModel.count; ai++)
                                        alts.push(Number(routeModel.get(ai).altitude))
                                    var profileNodes = []
                                    for (var wi = 0; wi < alts.length; wi++) {
                                        var boundMandatory = null
                                        for (var bm = 0; bm < root.mandatoryPoints.length; bm++) {
                                            if (Number(root.mandatoryPoints[bm].routeIndex) === wi
                                                && Number(root.mandatoryPoints[bm].routeIndex) >= 0) {
                                                boundMandatory = root.mandatoryPoints[bm]
                                                break
                                            }
                                        }
                                        profileNodes.push({
                                            progress: boundMandatory ? Number(boundMandatory.progress) : wi / Math.max(1, alts.length - 1),
                                            altitude: boundMandatory ? Number(boundMandatory.altitude) : alts[wi],
                                            waypoint: wi,
                                            mandatory: boundMandatory !== null,
                                            mandatoryId: boundMandatory ? boundMandatory.id : ""
                                        })
                                    }
                                    for (var mi = 0; mi < root.mandatoryPoints.length; mi++) {
                                        var mp = root.mandatoryPoints[mi]
                                        if (Number(mp.routeIndex) >= 0) continue
                                        profileNodes.push({ progress: Math.max(0, Math.min(1, Number(mp.progress))),
                                                            altitude: Math.max(0, Math.min(400, Number(mp.altitude))),
                                                            mandatory: true, mandatoryId: mp.id })
                                    }
                                    profileNodes.sort(function(a, b) { return a.progress - b.progress })
                                    for (var pn = 0; pn < profileNodes.length; pn++)
                                        profileNodes[pn].routeNumber = pn + 1
                                    ctx.beginPath()
                                    for (var p = 0; p < profileNodes.length; p++) {
                                        var px = left + plotW * profileNodes[p].progress
                                        var py = bottom - (profileNodes[p].altitude / 400) * plotH
                                        if (p === 0) ctx.moveTo(px, py); else ctx.lineTo(px, py)
                                    }
                                    ctx.strokeStyle = root.cyan; ctx.lineWidth = 3; ctx.stroke()
                                    // Number every route node in the same sequence as the table.
                                    for (var rn = 0; rn < profileNodes.length; rn++) {
                                        var routeNode = profileNodes[rn]
                                        var labelX = left + plotW * routeNode.progress
                                        var labelY = bottom - (routeNode.altitude / 400) * plotH - 17
                                        ctx.beginPath(); ctx.arc(labelX, labelY, 8, 0, Math.PI * 2)
                                        ctx.fillStyle = routeNode.mandatory ? "#155BFF" : "#071321"
                                        ctx.fill()
                                        ctx.strokeStyle = routeNode.mandatory ? "#8DB7FF" : root.cyan
                                        ctx.lineWidth = 1.5; ctx.stroke()
                                        ctx.fillStyle = "#FFFFFF"; ctx.font = "bold 10px sans-serif"
                                        ctx.textAlign = "center"; ctx.textBaseline = "middle"
                                        ctx.fillText(String(routeNode.routeNumber), labelX, labelY)
                                    }
                                    ctx.textAlign = "start"; ctx.textBaseline = "alphabetic"
                                    for (var m = 0; m < alts.length; m++) {
                                        var nodeProgress = m / Math.max(1, alts.length - 1)
                                        var nodeAltitude = alts[m]
                                        for (var bm2 = 0; bm2 < root.mandatoryPoints.length; bm2++) {
                                            if (Number(root.mandatoryPoints[bm2].routeIndex) === m
                                                && Number(root.mandatoryPoints[bm2].routeIndex) >= 0) {
                                                nodeProgress = Number(root.mandatoryPoints[bm2].progress)
                                                nodeAltitude = Number(root.mandatoryPoints[bm2].altitude)
                                                break
                                            }
                                        }
                                        var mx = left + plotW * nodeProgress
                                        var my = bottom - (nodeAltitude / 400) * plotH
                                        ctx.beginPath(); ctx.arc(mx, my, 4, 0, Math.PI * 2)
                                        ctx.fillStyle = "#EAF7FF"; ctx.fill()
                                        ctx.strokeStyle = root.cyan; ctx.lineWidth = 2; ctx.stroke()
                                    }
                                    for (var mi2 = 0; mi2 < root.mandatoryPoints.length; mi2++) {
                                        var mandatory = root.mandatoryPoints[mi2]
                                        var mandatoryX = left + plotW * Number(mandatory.progress)
                                        var mandatoryY = bottom - (Number(mandatory.altitude) / 400) * plotH
                                        ctx.strokeStyle = "#155BFF"; ctx.lineWidth = 2; ctx.setLineDash([4, 3])
                                        ctx.beginPath(); ctx.moveTo(mandatoryX, mandatoryY + 10); ctx.lineTo(mandatoryX, bottom); ctx.stroke()
                                        ctx.setLineDash([])
                                        // The draggable marker itself is a QML overlay above this Canvas.
                                        // Canvas retains the guide line and label only.
                                        ctx.fillStyle = "#FFFFFF"; ctx.font = "bold 11px sans-serif"
                                        ctx.fillText("ОБЯЗАТЕЛЬНАЯ", Math.min(right - 105, mandatoryX + 13), Math.max(top + 13, mandatoryY - 25))
                                    }
                                    ctx.fillStyle = "#DCE8F2"; ctx.font = "11px sans-serif"
                                    ctx.fillText("Высота, м", 3, 12)
                                    // Time axis: elapsed mission time, from departure to planned ETA.
                                    ctx.fillText("Время полёта", Math.max(45, width / 2 - 35), height - 4)
                                    ctx.fillText("00:00", left - 18, bottom + 14)
                                    ctx.fillText("400", left - 32, top + 4)
                                    ctx.fillText("01:18", right - 24, bottom + 14)
                                    for (var ti = 1; ti < 8; ti++) {
                                        var totalMinutes = Math.floor(78 * ti / 8)
                                        var hours = Math.floor(totalMinutes / 60)
                                        var minutes = totalMinutes % 60
                                        var label = (hours < 10 ? "0" : "") + hours + ":" +
                                                    (minutes < 10 ? "0" : "") + minutes
                                        ctx.fillText(label, left + plotW * ti / 8 - 15, bottom + 14)
                                    }

                                    // Aircraft marker: live telemetry takes priority. In the
                                    // design preview, show a clearly marked sample position.
                                    if (root.liveFlightActive || root.showAircraftPreview) {
                                        var markerDistance = root.liveFlightActive ? root.liveDistanceKm : root.previewDistanceKm
                                        var progress = Math.max(0, Math.min(1, markerDistance / root.plannedDistanceKm))
                                        var seg = progress * (alts.length - 1)
                                        var segIndex = Math.min(alts.length - 2, Math.floor(seg))
                                        var frac = seg - segIndex
                                        var routeAlt = alts[segIndex] + (alts[segIndex + 1] - alts[segIndex]) * frac
                                        var markerAlt = root.liveFlightActive && root.liveAltitudeM > 0
                                                        ? root.liveAltitudeM
                                                        : (root.liveFlightActive ? routeAlt : root.previewAltitudeM)
                                        var aircraftX = left + plotW * progress
                                        var aircraftY = bottom - (markerAlt / 400) * plotH

                                        // Halo and vertical leader make the aircraft easy to locate.
                                        ctx.strokeStyle = "#FFFFFF"
                                        ctx.lineWidth = 1
                                        ctx.setLineDash([3, 3])
                                        ctx.beginPath()
                                        ctx.moveTo(aircraftX, aircraftY + 13)
                                        ctx.lineTo(aircraftX, bottom)
                                        ctx.stroke()
                                        ctx.setLineDash([])
                                        ctx.beginPath()
                                        ctx.arc(aircraftX, aircraftY, 13, 0, Math.PI * 2)
                                        ctx.fillStyle = "#00DDF2"
                                        ctx.fill()
                                        ctx.strokeStyle = "#FFFFFF"
                                        ctx.lineWidth = 2
                                        ctx.stroke()

                                        ctx.save()
                                        ctx.translate(aircraftX, aircraftY)
                                        ctx.rotate(-Math.PI / 2)
                                        ctx.fillStyle = "#07111E"
                                        ctx.strokeStyle = "#FFFFFF"
                                        ctx.lineWidth = 1.5
                                        ctx.beginPath()
                                        ctx.moveTo(0, -9)
                                        ctx.lineTo(6, 7)
                                        ctx.lineTo(0, 4)
                                        ctx.lineTo(-6, 7)
                                        ctx.closePath()
                                        ctx.fill()
                                        ctx.stroke()
                                        ctx.restore()

                                        ctx.fillStyle = "#FFFFFF"
                                        ctx.font = "bold 11px sans-serif"
                                        var aircraftLabel = root.liveFlightActive ? "БПЛА" : "БПЛА · ПРИМЕР"
                                        var labelWidth = ctx.measureText(aircraftLabel).width
                                        var labelX = Math.max(left, Math.min(right - labelWidth, aircraftX + 16))
                                        var labelY = Math.max(top + 14, aircraftY - 17)
                                        ctx.fillStyle = "#07111E"
                                        ctx.fillRect(labelX - 3, labelY - 11, labelWidth + 6, 16)
                                        ctx.fillStyle = "#FFFFFF"
                                        ctx.fillText(aircraftLabel, labelX, labelY)

                                        if (root.liveFlightActive) {
                                            var liveTime = Math.max(0, Math.floor(root.liveElapsedSeconds))
                                            var liveLabel = (Math.floor(liveTime / 3600) < 10 ? "0" : "") + Math.floor(liveTime / 3600) + ":" +
                                                            (Math.floor((liveTime % 3600) / 60) < 10 ? "0" : "") + Math.floor((liveTime % 3600) / 60)
                                            ctx.font = "10px sans-serif"
                                            ctx.fillText(liveLabel, Math.min(right - 35, aircraftX + 10), Math.max(top + 28, aircraftY + 25))
                                        }
                                    }
                                }

                                MouseArea {
                                    id: mandatoryPointMouse
                                    // MouseArea is a child of Canvas: use Canvas-local coordinates.
                                    x: 0
                                    y: 0
                                    width: parent.width
                                    height: parent.height
                                    // Keep this MouseArea alive above the Repeater handles.
                                    // Repeater delegates are recreated when mandatoryPoints changes.
                                    z: 30
                                    hoverEnabled: true
                                    preventStealing: true
                                    cursorShape: pressed ? Qt.ClosedHandCursor : Qt.CrossCursor
                                    property int dragMandatoryIndex: -1
                                    property real plotLeft: 42
                                    property real plotRight: width - 12
                                    property real plotTop: 18
                                    property real plotBottom: height - 28
                                    property real dragOffsetX: 0
                                    property real dragOffsetY: 0

                                    function mandatoryPointAt(x, y) {
                                        var best = -1, bestD = 18 * 18
                                        for (var i = 0; i < root.mandatoryPoints.length; i++) {
                                            var px = plotLeft + Number(root.mandatoryPoints[i].progress) * (plotRight - plotLeft)
                                            var py = plotBottom - Number(root.mandatoryPoints[i].altitude) / 400 * (plotBottom - plotTop)
                                            var dx = x - px, dy = y - py, d = dx * dx + dy * dy
                                            if (d <= bestD) { best = i; bestD = d }
                                        }
                                        return best
                                    }
                                    function routePointAt(x, y) {
                                        // Match the visible route nodes with a forgiving touch/mouse hit area.
                                        var best = -1, bestD = 28 * 28
                                        for (var i = 0; i < routeModel.count; i++) {
                                            var px = plotLeft + i / Math.max(1, routeModel.count - 1) * (plotRight - plotLeft)
                                            var py = plotBottom - Number(routeModel.get(i).altitude) / 400 * (plotBottom - plotTop)
                                            var dx = x - px, dy = y - py, d = dx * dx + dy * dy
                                            if (d <= bestD) { best = i; bestD = d }
                                        }
                                        return best
                                    }

                                    function nearestRouteProgress(x, y) {
                                        // Project the pointer onto every visible profile segment.
                                        var bestProgress = -1, bestDistance = 28 * 28
                                        for (var i = 0; i < routeModel.count - 1; i++) {
                                            var x1 = plotLeft + i / Math.max(1, routeModel.count - 1) * (plotRight - plotLeft)
                                            var x2 = plotLeft + (i + 1) / Math.max(1, routeModel.count - 1) * (plotRight - plotLeft)
                                            var y1 = plotBottom - Number(routeModel.get(i).altitude) / 400 * (plotBottom - plotTop)
                                            var y2 = plotBottom - Number(routeModel.get(i + 1).altitude) / 400 * (plotBottom - plotTop)
                                            var dx = x2 - x1, dy = y2 - y1
                                            var t = Math.max(0, Math.min(1, ((x-x1)*dx + (y-y1)*dy) / Math.max(1, dx*dx + dy*dy)))
                                            var qx = x1 + t*dx, qy = y1 + t*dy
                                            var ex = x-qx, ey = y-qy, d = ex*ex + ey*ey
                                            if (d < bestDistance) {
                                                bestDistance = d
                                                bestProgress = (i+t) / Math.max(1, routeModel.count - 1)
                                            }
                                        }
                                        return bestProgress
                                    }
                                    function addMandatoryPoint(progress, altitude, routeIndex) {
                                        var next = root.mandatoryPoints.slice(0)
                                        next.push({ id: "mandatory-" + Date.now().toString() + "-" + next.length,
                                            progress: Math.max(0, Math.min(1, progress)),
                                            altitude: Math.round(Math.max(0, Math.min(400, altitude))),
                                            routeIndex: routeIndex === undefined ? -1 : routeIndex })
                                        root.mandatoryPoints = next
                                        return next.length - 1
                                    }

                                    function updateMandatoryPoint(index, mouseX, mouseY) {
                                        if (index < 0 || index >= root.mandatoryPoints.length) return
                                        var next = root.mandatoryPoints.slice(0)
                                        var selected = next[index]
                                        var nextProgress = Math.max(0, Math.min(1, (mouseX - plotLeft) / Math.max(1, plotRight - plotLeft)))
                                        var nextAltitude = Math.round(Math.max(0, Math.min(400, (plotBottom - mouseY) / Math.max(1, plotBottom - plotTop) * 400)))
                                        next[index] = {
                                            id: selected.id,
                                            progress: nextProgress,
                                             altitude: nextAltitude,
                                             distance: Number(distanceAtProgress(nextProgress).toFixed(1)),
                                            routeIndex: selected.routeIndex === undefined ? -1 : Number(selected.routeIndex)
                                        }
                                        var boundIndex = Number(selected.routeIndex)
                                        if (boundIndex >= 0 && boundIndex < routeModel.count)
                                            routeModel.setProperty(boundIndex, "altitude", String(nextAltitude))
                                        root.mandatoryPoints = next
                                        root.rebuildTableRows()
                                        profileCanvas.requestPaint()
                                    }

                                    function removeMandatoryPoint(index) {
                                        if (index < 0 || index >= root.mandatoryPoints.length) return
                                        var next = root.mandatoryPoints.slice(0)
                                        next.splice(index, 1)
                                        root.mandatoryPoints = next
                                    }

                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                    onPressed: {
                                        if (mouse.x < plotLeft || mouse.x > plotRight || mouse.y < plotTop || mouse.y > plotBottom) {
                                            mouse.accepted = false
                                            return
                                        }
                                        var hit = mandatoryPointAt(mouse.x, mouse.y)
                                        if (mouse.button === Qt.RightButton) {
                                            if (hit >= 0) removeMandatoryPoint(hit)
                                            else mouse.accepted = false
                                            return
                                        }
                                        if (hit >= 0) {
                                            dragMandatoryIndex = hit
                                            dragOffsetX = mouse.x - (plotLeft + Number(root.mandatoryPoints[hit].progress) * (plotRight - plotLeft))
                                            dragOffsetY = mouse.y - (plotBottom - Number(root.mandatoryPoints[hit].altitude) / 400 * (plotBottom - plotTop))
                                            return
                                        }

                                        var routeHit = routePointAt(mouse.x, mouse.y)
                                        if (routeHit >= 0) {
                                            var routeProgress = routeHit / Math.max(1, routeModel.count - 1)
                                            var routeAltitude = Number(routeModel.get(routeHit).altitude)
                                            var exists = false
                                            for (var r = 0; r < root.mandatoryPoints.length; r++)
                                                if (Math.abs(Number(root.mandatoryPoints[r].progress) - routeProgress) < 0.012) exists = true
                                            if (!exists) {
                                                dragMandatoryIndex = addMandatoryPoint(routeProgress, routeAltitude, routeHit)
                                                dragOffsetX = mouse.x - (plotLeft + routeProgress * (plotRight - plotLeft))
                                                dragOffsetY = mouse.y - (plotBottom - routeAltitude / 400 * (plotBottom - plotTop))
                                            }
                                            return
                                        }
                                        var progress = nearestRouteProgress(mouse.x, mouse.y)
                                        if (progress < 0) {
                                            mouse.accepted = false
                                            return
                                        }
                                        dragMandatoryIndex = addMandatoryPoint(progress,
                                            (plotBottom - mouse.y) / Math.max(1, plotBottom - plotTop) * 400)
                                        dragOffsetX = 0
                                        dragOffsetY = 0
                                    }
                                    onPositionChanged: {
                                        if (dragMandatoryIndex >= 0)
                                            updateMandatoryPoint(dragMandatoryIndex, mouse.x - dragOffsetX, mouse.y - dragOffsetY)
                                    }
                                    onReleased: dragMandatoryIndex = -1
                                    onDoubleClicked: {
                                        var hit = mandatoryPointAt(mouse.x, mouse.y)
                                        if (hit >= 0) removeMandatoryPoint(hit)
                                    }
                                }

                                // Visual marker delegates only. All pointer input is handled by
                                // the persistent mandatoryPointMouse above.
                                Repeater {
                                    id: mandatoryPointHandles
                                    model: root.mandatoryPoints
                                    delegate: Item {
                                        id: mandatoryHandle
                                        width: 22
                                        height: 22
                                        // Interactive handle is aligned to the exact Canvas plot geometry.
                                        z: 20
                                        x: 42 + Number(modelData.progress) * (profileCanvas.width - 54) - width / 2
                                        y: profileCanvas.height - 28
                                           - (Number(modelData.altitude) / 400) * (profileCanvas.height - 46)
                                           - height / 2

                                        Rectangle {
                                            width: 10
                                            height: 10
                                            anchors.centerIn: parent
                                            radius: width / 2
                                            color: "#155BFF"
                                            border.color: "#B8D4FF"
                                            border.width: 1.5
                                        }
                                    }
                                }
                            }

                        }
                    }
                }

                Rectangle {
                    id: parameterPanel
                    z: 40
                    width: root.parameterPanelOpen
                           ? Math.min(300, Math.max(220, contentRow.width * 0.18))
                           : 0
                    height: root.parameterPanelOpen
                           ? Math.min(parent.height, 62 + 12 * 32 + 11 * 4)
                           : 0
                    color: root.bg
                    border.color: root.line
                    radius: 3
                    Behavior on width { NumberAnimation { duration: 160 } }

                    Column {
                        anchors.fill: parent
                        anchors.margins: root.parameterPanelOpen ? 8 : 0
                        spacing: 8

                        Item {
                            width: parent.width
                            height: 38

                            Text {
                                visible: root.parameterPanelOpen
                                anchors.left: parent.left
                                anchors.right: panelToggle.left
                                anchors.rightMargin: 4
                                anchors.verticalCenter: parent.verticalCenter
                                text: "ОТОБРАЖЕНИЕ ПАРАМЕТРОВ"
                                color: root.textColor
                                font.family: "B612"
                                font.pixelSize: 14
                                font.bold: true
                                wrapMode: Text.Wrap
                                verticalAlignment: Text.AlignVCenter
                            }

                            Text {
                                id: panelToggle
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                width: 28
                                height: 32
                                text: "☰"
                                color: root.parameterPanelOpen ? "#FFFFFF" : root.cyan
                                font.pixelSize: 22
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter

                                MouseArea {
                                    anchors.fill: parent
                                    anchors.margins: -4
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.parameterPanelOpen = !root.parameterPanelOpen
                                }
                            }
                        }

                        Rectangle {
                            visible: root.parameterPanelOpen
                            width: parent.width
                            height: 1
                            color: root.line
                        }

                        ListView {
                            id: parameterListView
                            visible: root.parameterPanelOpen
                            width: parent.width
                            height: Math.max(0, parent.height - 55)
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds
                            spacing: 4
                            model: [
                                 "№", "Тип", "Точка / координаты", "Курс", "Дистанция",
                                 "Высота", "V_возд", "V_пут", "Время",
                                 "Δh (набор/снижение)", "Энергия", "Примечание"
                             ]

                            delegate: Item {
                                id: parameterRow
                                width: parameterListView.width
                                height: 34

                                property string parameterKey: ["number", "type", "point", "course", "distance", "altitude",
                                                               "airspeed", "groundspeed", "time", "deltaHeight", "energy", "note"][index]
                                property bool parameterChecked: root.parameterVisibility[parameterKey] !== false

                                Rectangle {
                                    id: parameterSwitch
                                    x: 0
                                    y: (parameterRow.height - height) / 2
                                    width: 38
                                    height: 21
                                    radius: 11
                                    color: parameterRow.parameterChecked ? root.switchGreen : "#263847"
                                    border.width: 1
                                    border.color: parameterRow.parameterChecked ? root.switchGreen : "#547084"

                                    Rectangle {
                                        width: 15
                                        height: 15
                                        radius: 8
                                        y: (parameterSwitch.height - height) / 2
                                        x: parameterRow.parameterChecked ? parameterSwitch.width - width - 3 : 3
                                        color: parameterRow.parameterChecked ? "#07111E" : "#B7C7D3"
                                    }
                                }

                                Text {
                                    id: parameterLabel
                                    x: 48
                                    y: (parameterRow.height - height) / 2
                                    width: Math.max(0, parameterRow.width - 48)
                                    text: modelData
                                    color: root.textColor
                                    font.family: "B612"
                                    font.pixelSize: 16
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignLeft
                                    wrapMode: Text.NoWrap
                                    elide: Text.ElideRight
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.toggleParameter(parameterRow.parameterKey)
                                }
                            }

                        }
                    }
                }
            }
        }

    ListModel {
        id: routeModel
        ListElement { pointType: "Старт"; pointName: "WP0 (База)"; coordinates: "55.7522, 37.6156"; course: "—"; distance: "0.0"; altitude: "120"; airspeed: "—"; groundspeed: "—"; time: "00:00"; deltaHeight: "—"; energy: "100"; note: "Взлёт (VTOL)" }
        ListElement { pointType: "Участок"; pointName: "WP1"; coordinates: "55.7801, 37.6923"; course: "087"; distance: "12.4"; altitude: "150"; airspeed: "28.0"; groundspeed: "25.4"; time: "04:52"; deltaHeight: "+30"; energy: "92"; note: "Набор высоты" }
        ListElement { pointType: "Участок"; pointName: "WP2"; coordinates: "55.8065, 37.8041"; course: "095"; distance: "18.7"; altitude: "150"; airspeed: "30.0"; groundspeed: "27.1"; time: "06:54"; deltaHeight: "0"; energy: "81"; note: "Патрулирование" }
        ListElement { pointType: "Участок"; pointName: "WP3"; coordinates: "55.8410, 37.9124"; course: "110"; distance: "15.6"; altitude: "180"; airspeed: "28.5"; groundspeed: "26.0"; time: "05:47"; deltaHeight: "+30"; energy: "68"; note: "Обход зоны" }
        ListElement { pointType: "Участок"; pointName: "WP4"; coordinates: "55.8702, 38.0231"; course: "132"; distance: "16.8"; altitude: "150"; airspeed: "29.0"; groundspeed: "24.8"; time: "06:46"; deltaHeight: "-30"; energy: "54"; note: "Съёмка" }
        ListElement { pointType: "Финиш"; pointName: "WP5 (Цель)"; coordinates: "55.9001, 38.1156"; course: "142"; distance: "14.9"; altitude: "120"; airspeed: "28.0"; groundspeed: "25.6"; time: "05:59"; deltaHeight: "-30"; energy: "42"; note: "Снижение, посадка" }
    }
}
}
