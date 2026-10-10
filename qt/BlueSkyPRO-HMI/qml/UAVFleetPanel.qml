import QtQuick
import QtCore

Item {
    id: root

    signal uavSelected(int index)
    signal uavDoubleClicked(int index)

    property int selectedIndex: -1
    property int settingsIndex: -1
    property int dragSourceIndex: -1
    property real dragStartX: 0
    property real dragStartY: 0
    property int dragTargetIndex: -1
    property int expandedUavIndex: -1
    property string expandedParameter: ""
    property var uavModel: [
        { id: "BS-001", modelName: "MULTIROTOR", imageSource: "assets/uav_multirotor.svg", sequence: "01", state: "READY", missionTotalSeconds: 3725, missionRemainingSeconds: 3725, missionStarted: false, height: 0, speed: 0, battery: 100, engine: 0, wind: "—", heading: "—", eta: "—", c2: "C2 OK", camera: "—", range: "120 km", eet: "—", trip: "—", tot: "—", gnss: "3D FIX", link: "OK", wp: "—", progress: "STBY", batteryHealth: "100 %", payload: "—", telemetry: "NOMINAL" },
        { id: "BS-002", modelName: "FIXED WING", imageSource: "assets/uav_fixed_wing.svg", sequence: "02", state: "STBY", missionTotalSeconds: 5400, missionRemainingSeconds: 5400, missionStarted: false, height: 0, speed: 0, battery: 100, engine: 0, wind: "—", heading: "—", eta: "—", c2: "C2 OK", camera: "—", range: "180 km", eet: "—", trip: "—", tot: "—", gnss: "3D FIX", link: "OK", wp: "—", progress: "STBY", batteryHealth: "100 %", payload: "—", telemetry: "NOMINAL" },
        { id: "BS-003", modelName: "MULTIROTOR", imageSource: "assets/uav_heavy_multirotor.svg", sequence: "03", state: "READY", missionTotalSeconds: 2700, missionRemainingSeconds: 2700, missionStarted: false, height: 0, speed: 0, battery: 100, engine: 0, wind: "—", heading: "—", eta: "—", c2: "C2 OK", camera: "—", range: "150 km", eet: "—", trip: "—", tot: "—", gnss: "3D FIX", link: "OK", wp: "—", progress: "STBY", batteryHealth: "100 %", payload: "—", telemetry: "NOMINAL" },
        { id: "BS-004", modelName: "VTOL", imageSource: "assets/uav_vtol.svg", sequence: "04", state: "READY", missionTotalSeconds: 3599, missionRemainingSeconds: 3599, missionStarted: false, height: 0, speed: 0, battery: 100, engine: 0, wind: "—", heading: "—", eta: "—", c2: "C2 OK", camera: "—", range: "200 km", eet: "—", trip: "—", tot: "—", gnss: "3D FIX", link: "OK", wp: "—", progress: "STBY", batteryHealth: "100 %", payload: "—", telemetry: "NOMINAL" }
    ]

    property var defaultParameters: ["ALT", "SPD", "BAT", "ENG"]
    readonly property int maxParameterRows: {
        var count = root.defaultParameters.length
        for (var i = 0; i < root.uavModel.length; ++i)
            count = Math.max(count, root.parametersFor(i).length)
        return count
    }
    readonly property int preferredHeight: 48 + root.maxParameterRows * 20 + 42 + 16
    property var parameterConfigs: ({})

    readonly property var availableParameterOptions: [
        { key: "ALT", label: "Высота (HGT / ALT)" },
        { key: "SPD", label: "Скорость" },
        { key: "BAT", label: "Battery, %" },
        { key: "ENG", label: "Режим двигателей" },
        { key: "WIND", label: "Ветер" },
        { key: "HDG", label: "Курс" },
        { key: "ETA", label: "ETA — расчётное прибытие" },
        { key: "C2", label: "C2 / канал управления" },
        { key: "CAM", label: "Камера" },
        { key: "RNG", label: "Остаточная дальность" },
        { key: "EET", label: "EET — расчётное время полёта" },
        { key: "TRIP", label: "Trip Time" },
        { key: "TOT", label: "TOT — время взлёта" },
        { key: "GNSS", label: "GNSS / навигация" },
        { key: "LINK", label: "Состояние связи" },
        { key: "WP", label: "Waypoint" },
        { key: "PROGRESS", label: "Прогресс миссии" },
        { key: "BAT HEALTH", label: "Состояние аккумулятора" },
        { key: "PAYLOAD", label: "Полезная нагрузка" },
        { key: "TELEM", label: "Качество телеметрии" }
    ]

    Settings {
        id: settings
        category: "BlueSkyPRO/UAVPanel"
        property string uavOrderJson: ""
        property string parameterConfigsJson: ""
        property real settingsPopupX: -1
        property real settingsPopupY: -1
    }

    Timer {
        id: missionCountdown
        interval: 1000
        repeat: true
        running: true
        onTriggered: {
            var next = root.uavModel.map(function(item) {
                var copy = Object.assign({}, item)
                if (copy.missionStarted === true)
                    copy.missionRemainingSeconds = Math.max(0, Number(copy.missionRemainingSeconds) - 1)
                return copy
            })
            root.uavModel = next
        }
    }

    function parseJson(value, fallback) {
        try { return value ? JSON.parse(value) : fallback } catch (e) { return fallback }
    }

    function parametersFor(index) {
        if (index < 0 || index >= root.uavModel.length)
            return root.defaultParameters.slice()
        var saved = root.parameterConfigs[root.uavModel[index].id]
        return Array.isArray(saved) ? saved.slice() : root.defaultParameters.slice()
    }

    // Card geometry in UAVFleetPanel coordinates, matching the form's responsive grid.
    function cardRect(index) {
        if (index < 0 || index >= root.uavModel.length)
            return Qt.rect(0, 0, 0, 0)

        var columns = Math.max(1, form.gridColumns)
        var gap = form.gridSpacing
        var cardWidth = form.gridCardWidth
        var cardHeight = form.gridCardHeight
        var row = Math.floor(index / columns)
        var column = index % columns
        var itemsInRow = Math.min(columns, root.uavModel.length - row * columns)
        var gridWidth = Math.max(0, root.width - 20)
        var rowWidth = itemsInRow * cardWidth + (itemsInRow - 1) * gap
        var cardX = 10 + Math.max(0, (gridWidth - rowWidth) / 2)
                    + column * (cardWidth + gap)
        var rowCount = Math.ceil(root.uavModel.length / columns)
        var gridHeight = rowCount * cardHeight + Math.max(0, rowCount - 1) * gap
        var cardY = root.height - gridHeight + row * (cardHeight + gap)
        return Qt.rect(cardX, cardY, cardWidth, cardHeight)
    }

    function parameterLabel(key, data) {
        if (key === "ALT") return data.height < 100 ? "HGT" : "ALT"
        if (key === "SPD") return "SPD"
        if (key === "BAT") return "BAT"
        if (key === "ENG") return "ENG"
        if (key === "WIND") return "WIND"
        if (key === "HDG") return "HDG"
        if (key === "ETA") return "ETA"
        if (key === "C2") return "C2"
        if (key === "CAM") return "CAM"
        if (key === "RNG") return "RNG"
        if (key === "EET") return "EET"
        if (key === "TRIP") return "TRIP"
        if (key === "TOT") return "TOT"
        if (key === "GNSS") return "GNSS"
        if (key === "LINK") return "LINK"
        if (key === "WP") return "WP"
        if (key === "PROGRESS") return "MISSION"
        if (key === "BAT HEALTH") return "BAT HLTH"
        if (key === "PAYLOAD") return "LOAD"
        if (key === "TELEM") return "TELEM"
        return key
    }

    function parameterValue(key, data) {
        if (key === "ALT") return Math.round(data.height) + " m"
        if (key === "SPD") return data.speed + " km/h"
        if (key === "BAT") return data.battery + " %"
        if (key === "ENG") return data.engine + " %"
        if (key === "WIND") return data.wind
        if (key === "HDG") return data.heading + "°"
        if (key === "ETA") return data.eta
        if (key === "C2") return data.c2
        if (key === "CAM") return data.camera
        if (key === "RNG") return data.range
        if (key === "EET") return data.eet
        if (key === "TRIP") return data.trip
        if (key === "TOT") return data.tot
        if (key === "GNSS") return data.gnss
        if (key === "LINK") return data.link
        if (key === "WP") return data.wp
        if (key === "PROGRESS") return data.progress
        if (key === "BAT HEALTH") return data.batteryHealth
        if (key === "PAYLOAD") return data.payload
        if (key === "TELEM") return data.telemetry
        return "—"
    }

    readonly property var displayModel: root.uavModel.map(function(data, index) {
        var enriched = Object.assign({}, data)
        var keys = root.parametersFor(index)
        // Every enabled parameter is rendered as a readable row.
        // The card height grows with the configured row count.
        enriched.primaryParameters = keys.map(function(key) {
            return { key: key, label: root.parameterLabel(key, data), value: root.parameterValue(key, data) }
        })
        enriched.smartTools = []
        return enriched
    })
    readonly property string expandedParameterLabel: root.expandedUavIndex >= 0
        ? root.parameterLabel(root.expandedParameter, root.uavModel[root.expandedUavIndex]) : ""
    readonly property string expandedParameterValue: root.expandedUavIndex >= 0
        ? root.parameterValue(root.expandedParameter, root.uavModel[root.expandedUavIndex]) : ""

    function persist() {
        settings.uavOrderJson = JSON.stringify(root.uavModel.map(function(item) { return item.id }))
        settings.parameterConfigsJson = JSON.stringify(root.parameterConfigs)
    }

    function moveUav(fromIndex, toIndex) {
        if (fromIndex < 0 || toIndex < 0 || fromIndex >= root.uavModel.length || toIndex >= root.uavModel.length || fromIndex === toIndex)
            return
        var next = root.uavModel.slice()
        var item = next.splice(fromIndex, 1)[0]
        next.splice(toIndex, 0, item)
        root.uavModel = next
        if (root.selectedIndex === fromIndex)
            root.selectedIndex = toIndex
        else if (fromIndex < root.selectedIndex && toIndex >= root.selectedIndex)
            root.selectedIndex -= 1
        else if (fromIndex > root.selectedIndex && toIndex <= root.selectedIndex)
            root.selectedIndex += 1
        root.persist()
    }

    function toggleParameter(index, parameter) {
        if (index < 0 || index >= root.uavModel.length)
            return
        var next = root.parametersFor(index)
        var at = next.indexOf(parameter)
        if (at >= 0)
            next.splice(at, 1)
        else {
            if (next.length >= 8)
                return
            next.push(parameter)
        }
        var configs = Object.assign({}, root.parameterConfigs)
        configs[root.uavModel[index].id] = next
        root.parameterConfigs = configs
        root.persist()
    }

    function moveParameter(index, parameter, direction) {
        if (index < 0 || index >= root.uavModel.length)
            return
        var next = root.parametersFor(index)
        var at = next.indexOf(parameter)
        var target = at + direction
        if (at < 0 || target < 0 || target >= next.length)
            return
        var item = next.splice(at, 1)[0]
        next.splice(target, 0, item)
        var configs = Object.assign({}, root.parameterConfigs)
        configs[root.uavModel[index].id] = next
        root.parameterConfigs = configs
        root.persist()
    }

    function applyParametersToAll() {
        var params = root.parametersFor(root.settingsIndex)
        var configs = Object.assign({}, root.parameterConfigs)
        for (var i = 0; i < root.uavModel.length; ++i)
            configs[root.uavModel[i].id] = params.slice()
        root.parameterConfigs = configs
        root.persist()
    }

    function finishDrag() {
        if (root.dragSourceIndex >= 0 && root.dragTargetIndex >= 0)
            root.moveUav(root.dragSourceIndex, root.dragTargetIndex)
        root.dragSourceIndex = -1
        root.dragTargetIndex = -1
        form.dragIndex = -1
        form.dragOffsetX = 0
        form.dragOffsetY = 0
    }

    Component.onCompleted: {
        root.parameterConfigs = root.parseJson(settings.parameterConfigsJson, ({}))
        form.settingsPopupX = settings.settingsPopupX
        form.settingsPopupY = settings.settingsPopupY
        var savedOrder = root.parseJson(settings.uavOrderJson, [])
        if (Array.isArray(savedOrder) && savedOrder.length) {
            var ordered = []
            for (var i = 0; i < savedOrder.length; ++i) {
                for (var j = 0; j < root.uavModel.length; ++j)
                    if (root.uavModel[j].id === savedOrder[i]) ordered.push(root.uavModel[j])
            }
            for (var k = 0; k < root.uavModel.length; ++k) {
                var found = false
                for (var n = 0; n < ordered.length; ++n)
                    if (ordered[n].id === root.uavModel[k].id) found = true
                if (!found) ordered.push(root.uavModel[k])
            }
            root.uavModel = ordered
        }
    }

    UAVFleetPanelForm {
        id: form
        anchors.fill: parent
        displayModel: root.displayModel
        selectedIndex: root.selectedIndex
        settingsIndex: root.settingsIndex
        settingsParameters: root.parametersFor(root.selectedIndex >= 0 ? root.selectedIndex : root.settingsIndex)
        settingsPopupX: settings.settingsPopupX
        settingsPopupY: settings.settingsPopupY
        expandedUavIndex: root.expandedUavIndex
        expandedParameterLabel: root.expandedParameterLabel
        expandedParameterValue: root.expandedParameterValue
        availableParameterOptions: root.availableParameterOptions

        onUavSelected: {
            root.selectedIndex = index
            if (form.settingsOpen)
                root.settingsIndex = index
            root.uavSelected(index)
        }
        onUavDoubleClicked: {
            root.selectedIndex = index
            root.uavDoubleClicked(index)
        }
        onSettingsRequested: {
            if (form.settingsOpen && root.settingsIndex === index) {
                form.settingsOpen = false
            } else {
                root.selectedIndex = index
                root.settingsIndex = index
                root.uavSelected(index)
                form.settingsOpen = true
            }
        }
        onSettingsPositionChanged: {
            settings.settingsPopupX = x
            settings.settingsPopupY = y
        }
        onDragStarted: {
            root.dragSourceIndex = index
            root.dragTargetIndex = index
            root.dragStartX = x
            root.dragStartY = y
            form.dragIndex = index
        }
        onDragMoved: {
            if (root.dragSourceIndex !== index)
                return
            form.dragOffsetX = x - root.dragStartX
            form.dragOffsetY = y - root.dragStartY
            var col = Math.max(0, Math.min(form.gridColumns - 1,
                Math.floor(x / (form.gridCardWidth + form.gridSpacing))))
            var row = Math.max(0, Math.floor(y / (form.gridCardHeight + form.gridSpacing)))
            root.dragTargetIndex = Math.max(0, Math.min(root.uavModel.length - 1,
                row * form.gridColumns + col))
            form.dragTargetIndex = root.dragTargetIndex
        }
        onDragFinished: root.finishDrag()
        onSmartToolRequested: {
            if (root.expandedUavIndex === index && root.expandedParameter === parameter) {
                root.expandedUavIndex = -1
                root.expandedParameter = ""
            } else {
                root.expandedUavIndex = index
                root.expandedParameter = parameter
            }
        }
        onParameterToggleRequested: root.toggleParameter(root.settingsIndex, parameter)
        onParameterMoveRequested: root.moveParameter(root.settingsIndex, parameter, direction)
        onApplyToAllRequested: root.applyParametersToAll()
        onSettingsClosed: form.settingsOpen = false
    }
}
