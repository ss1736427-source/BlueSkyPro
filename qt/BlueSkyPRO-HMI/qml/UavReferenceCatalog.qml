import QtQuick
import QtQuick.Controls
import "UavReferenceCatalog.js" as UavCatalog

Item {
    id: root
    readonly property var typeRecords: UavCatalog.types
    readonly property var modelRecords: UavCatalog.models
    property int selectedTypeIndex: 0
    property int selectedModelIndex: -1

    readonly property var selectedType: selectedTypeIndex >= 0 && selectedTypeIndex < typeRecords.length
        ? typeRecords[selectedTypeIndex] : null
    readonly property var visibleModels: selectedType ? UavCatalog.modelsForType(selectedType.id) : modelRecords

    readonly property color bg: "#07121D"
    readonly property color card: "#102333"
    readonly property color line: "#233B4D"
    readonly property color fg: "#EDF5FC"
    readonly property color muted: "#A2B4C6"
    readonly property color cyan: "#08B9F2"
    readonly property color amber: "#F5C542"

    Rectangle { anchors.fill: parent; color: root.bg }
    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 14

        Row {
            width: parent.width
            spacing: 12
            Column {
                width: parent.width - statusBadge.width - 12
                spacing: 4
                Text { text: "СПРАВОЧНИК БЕСПИЛОТНЫХ ВОЗДУШНЫХ СУДОВ"; color: root.fg; font.pixelSize: 21; font.bold: true }
                Text { text: "Типы платформ и справочные модели для планирования"; color: root.muted; font.pixelSize: 12 }
            }
            Rectangle {
                id: statusBadge
                width: 142; height: 30; radius: 5
                color: "#4A3B16"; border.color: "#8B7025"
                Text { anchors.centerIn: parent; text: "REFERENCE ONLY"; color: root.amber; font.pixelSize: 10; font.bold: true }
            }
        }

        Text {
            width: parent.width
            wrapMode: Text.Wrap
            color: root.muted
            font.pixelSize: 11
            text: "Справочные значения производителя. Не являются утверждённой конфигурацией конкретного борта или эксплуатационными ограничениями. Неизвестные параметры не подменяются расчётными догадками."
        }

        Row {
            width: parent.width
            height: 38
            spacing: 6
            Repeater {
                model: root.typeRecords
                delegate: Rectangle {
                    required property var modelData
                    required property int index
                    width: Math.max(82, (parent.width - (root.typeRecords.length - 1) * 6) / root.typeRecords.length)
                    height: 34; radius: 4
                    color: root.selectedTypeIndex === index ? "#07517A" : root.card
                    border.color: root.selectedTypeIndex === index ? root.cyan : root.line
                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 8
                        text: modelData.name
                        color: root.fg
                        font.pixelSize: 9
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { root.selectedTypeIndex = index; root.selectedModelIndex = -1 } }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: Math.min(140, typeInfo.implicitHeight + 24)
            radius: 7; color: root.card; border.color: root.line
            Column {
                id: typeInfo
                anchors.fill: parent; anchors.margins: 12; spacing: 6
                Text { text: root.selectedType ? root.selectedType.id + "  /  " + root.selectedType.nameEn : ""; color: root.cyan; font.pixelSize: 10; font.bold: true }
                Text { width: parent.width; text: root.selectedType ? root.selectedType.description : ""; color: root.fg; font.pixelSize: 12; wrapMode: Text.Wrap }
                Text { width: parent.width; text: root.selectedType ? "Возможности: " + root.selectedType.capabilities.join(" · ") : ""; color: root.muted; font.pixelSize: 10; wrapMode: Text.Wrap }
                Text { width: parent.width; text: root.selectedType ? "Условия планирования: " + root.selectedType.planningNotes : ""; color: root.amber; font.pixelSize: 10; wrapMode: Text.Wrap }
            }
        }

        Row {
            width: parent.width
            Text { text: "МОДЕЛИ В СПРАВОЧНИКЕ"; color: root.cyan; font.pixelSize: 11; font.bold: true }
            Item { width: Math.max(0, parent.width - 250); height: 1 }
            Text { text: root.visibleModels.length + " записей"; color: root.muted; font.pixelSize: 10 }
        }

        Flickable {
            width: parent.width
            height: Math.max(120, parent.height - 270)
            clip: true
            contentWidth: width
            contentHeight: modelColumn.implicitHeight
            Column {
                id: modelColumn
                width: parent.width
                spacing: 7
                Repeater {
                    model: root.visibleModels
                    delegate: Rectangle {
                        required property var modelData
                        width: modelColumn.width
                        height: modelInfo.implicitHeight + 20
                        radius: 6; color: root.card; border.color: root.line
                        Column {
                            id: modelInfo
                            anchors.fill: parent; anchors.margins: 10; spacing: 5
                            Row {
                                width: parent.width
                                Text { text: modelData.manufacturer + (modelData.manufacturerCountry === "RU" ? " · РФ" : "") + "  " + modelData.model; color: root.fg; font.pixelSize: 13; font.bold: true }
                                Item { width: Math.max(0, parent.width - 300); height: 1 }
                                Text { text: modelData.qualification; color: root.amber; font.pixelSize: 9 }
                            }
                            Text {
                                width: parent.width
                                color: root.muted; font.pixelSize: 10; wrapMode: Text.Wrap
                                text: modelData.category
                                    + (modelData.mtowKg !== undefined && modelData.mtowKg !== null ? "  ·  MTOW " + modelData.mtowKg + " кг" : "")
                                    + (modelData.maxSpeedMps !== undefined ? "  ·  Vmax " + modelData.maxSpeedMps + " м/с" : "")
                                    + (modelData.maxFlightTimeMin !== undefined ? "  ·  до " + modelData.maxFlightTimeMin + " мин (тест)" : "")
                            }
                            Text {
                                width: parent.width
                                color: root.fg; font.pixelSize: 10; wrapMode: Text.Wrap
                                text: "Оборудование: " + (modelData.payload || "не определено")
                                    + (modelData.battery ? "  ·  АКБ: " + modelData.battery : "")
                            }
                            Text { width: parent.width; text: "Примечание: " + modelData.note; color: root.amber; font.pixelSize: 10; wrapMode: Text.Wrap }
                            Text { width: parent.width; text: "Источник: " + modelData.source; color: root.cyan; font.pixelSize: 9; wrapMode: Text.Wrap }
                            Text { width: parent.width; text: "Неизвестно / не подтверждено: " + modelData.unknowns.join(", "); color: root.muted; font.pixelSize: 9; wrapMode: Text.Wrap }
                        }
                    }
                }
            }
        }
    }
}
