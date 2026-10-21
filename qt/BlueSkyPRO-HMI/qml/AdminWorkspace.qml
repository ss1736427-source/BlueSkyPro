import QtQuick
import QtQuick.Controls

Item {
    id: root
    property string activeSection: "Integrations"
    signal exitRequested()

    readonly property color bg: "#07121D"
    readonly property color sidebar: "#0D2030"
    readonly property color topbar: "#0B1B29"
    readonly property color card: "#102333"
    readonly property color line: "#233B4D"
    readonly property color fg: "#EDF5FC"
    readonly property color muted: "#A2B4C6"
    readonly property color cyan: "#08B9F2"
    readonly property color green: "#21D6A0"
    readonly property color amber: "#F5C542"

    function titleFor(key) {
        return ({
            Overview: "Обзор системы", Users: "Пользователи и роли",
            Roles: "Пользователи и роли", Security: "Системное администрирование",
            Settings: "Системное администрирование", Integrations: "Внешние интеграции",
            Sync: "Данные и обработка", Documents: "Документы и соответствие",
            Audit: "Журнал аудита", Diagnostics: "Обслуживание и диагностика"
        })[key] || "Администрирование"
    }

    Rectangle { anchors.fill: parent; color: root.bg }

    Row {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: navigation
            width: Math.min(252, Math.max(214, root.width * 0.20))
            height: parent.height
            color: root.sidebar
            border.color: root.line
            border.width: 1

            Column {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    width: parent.width
                    height: 78
                    color: "#0A1A28"
                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        spacing: 12
                        Item {
                            width: 30; height: parent.height
                            Text { anchors.centerIn: parent; text: "◢"; color: root.cyan; font.pixelSize: 28; font.bold: true }
                        }
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 3
                            Text { text: "BlueSky PRO"; color: root.cyan; font.pixelSize: 20; font.bold: true }
                            Text { text: "A D M I N I S T R A T O R"; color: root.muted; font.pixelSize: 8 }
                        }
                    }
                }

                Flickable {
                    width: parent.width
                    height: Math.max(0, parent.height - 126)
                    clip: true
                    contentWidth: width
                    contentHeight: navItems.implicitHeight

                    Column {
                        id: navItems
                        width: parent.width
                        spacing: 0

                        Repeater {
                            model: [
                                {key:"Overview", title:"Обзор системы", glyph:"⌂", group:""},
                                {key:"Users", title:"Пользователи и роли", glyph:"♟", group:""},
                                {key:"Security", title:"Системное администрирование", glyph:"⚙", group:""},
                                {key:"Integrations", title:"Внешние интеграции", glyph:"♆", group:""},
                                {key:"Sync", title:"Данные и обработка", glyph:"▤", group:""},
                                {key:"Documents", title:"Документы и соответствие", glyph:"▣", group:""},
                                {key:"Audit", title:"Журнал аудита", glyph:"◴", group:""},
                                {key:"Diagnostics", title:"Обслуживание и диагностика", glyph:"🔧", group:""}
                            ]
                            delegate: Rectangle {
                                required property var modelData
                                width: navItems.width
                                height: 50
                                color: root.activeSection === modelData.key ? "#07517A" : "transparent"
                                Rectangle {
                                    width: 4; height: parent.height
                                    color: root.activeSection === modelData.key ? root.cyan : "transparent"
                                }
                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 14
                                    anchors.rightMargin: 10
                                    spacing: 12
                                    Text {
                                        width: 26
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: modelData.glyph
                                        color: root.activeSection === modelData.key ? root.cyan : "#C1D2E1"
                                        font.pixelSize: 22
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    Text {
                                        width: parent.width - 54
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: modelData.title
                                        color: root.activeSection === modelData.key ? root.fg : "#D5E0EA"
                                        font.pixelSize: 12
                                        wrapMode: Text.Wrap
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.activeSection = modelData.key
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width; height: 48
                    color: "#0A1A28"
                    Row {
                        anchors.centerIn: parent
                        spacing: 12
                        Text { text: "‹‹"; color: root.muted; font.pixelSize: 18 }
                        Text { text: "Свернуть"; color: root.muted; font.pixelSize: 11 }
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: navigation.width = navigation.width === 0 ? 236 : 0
                    }
                }
            }
        }

        Item {
            id: contentArea
            width: Math.max(0, root.width - navigation.width)
            height: parent.height
            clip: true

            Rectangle {
                id: adminTopBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 50
                color: root.topbar
                border.color: root.line
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 16
                    spacing: 10
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "⌂"; color: root.cyan; font.pixelSize: 22 }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "Администратор"; color: root.muted; font.pixelSize: 11 }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "›"; color: root.muted; font.pixelSize: 18 }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: root.titleFor(root.activeSection)
                        color: root.fg; font.pixelSize: 12; font.bold: true
                        elide: Text.ElideRight
                    }
                    Item { width: Math.max(0, parent.width - 620); height: 1 }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "ADMINISTRATOR"; color: root.muted; font.pixelSize: 8; font.letterSpacing: 1 }
                    Rectangle {
                        width: 150; height: 34; radius: 6
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#17394E"; border.color: root.line
                        Text { anchors.centerIn: parent; text: "←  Выбор роли"; color: root.fg; font.pixelSize: 11; font.bold: true }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: root.exitRequested()
                        }
                    }
                }
            }

            Loader {
                anchors.fill: parent
                anchors.topMargin: adminTopBar.height
                active: root.activeSection === "Integrations"
                visible: active
                sourceComponent: Component {
                    ExternalIntegrationsWorkspace { anchors.fill: parent }
                }
            }

            Flickable {
                anchors.fill: parent
                anchors.topMargin: adminTopBar.height
                visible: root.activeSection !== "Integrations"
                clip: true
                contentWidth: width
                contentHeight: overviewPage.implicitHeight + 56

                Column {
                    id: overviewPage
                    x: 26; y: 26
                    width: Math.max(280, parent.width - 52)
                    spacing: 18

                    Text { text: root.titleFor(root.activeSection); color: root.fg; font.pixelSize: 28; font.bold: true }
                    Text {
                        width: parent.width
                        wrapMode: Text.Wrap
                        text: root.activeSection === "Overview"
                            ? "Состояние служб, подключений и критических сообщений системы."
                            : "Рабочая область администратора BlueSky PRO."
                        color: root.muted; font.pixelSize: 13
                    }

                    Grid {
                        width: parent.width
                        columns: width < 680 ? 1 : 3
                        spacing: 12
                        Repeater {
                            model: [
                                {title:"Внешние интеграции", desc:"NOTAM · WX · карты · рельеф", target:"Integrations"},
                                {title:"Пользователи и роли", desc:"Учётные записи и разрешения", target:"Users"},
                                {title:"Журнал аудита", desc:"Входы, изменения и события", target:"Audit"}
                            ]
                            delegate: Rectangle {
                                required property var modelData
                                width: (overviewPage.width - (overviewPage.width < 680 ? 0 : 24)) / (overviewPage.width < 680 ? 1 : 3)
                                height: 126; radius: 9; color: root.card; border.color: root.line
                                Column {
                                    anchors.fill: parent; anchors.margins: 16; spacing: 10
                                    Text { text: modelData.title; color: root.fg; font.pixelSize: 15; font.bold: true }
                                    Text { width: parent.width; text: modelData.desc; color: root.muted; font.pixelSize: 12; wrapMode: Text.Wrap }
                                    Text { text: "Открыть  →"; color: root.cyan; font.pixelSize: 11 }
                                }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.activeSection = modelData.target }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width; height: statusColumn.implicitHeight + 32
                        radius: 8; color: root.card; border.color: root.line
                        Column {
                            id: statusColumn
                            anchors.fill: parent; anchors.margins: 16; spacing: 10
                            Text { text: "СОСТОЯНИЕ СИСТЕМЫ"; color: root.cyan; font.pixelSize: 10; font.bold: true }
                            Text {
                                width: parent.width; wrapMode: Text.Wrap; color: root.fg; font.pixelSize: 12
                                text: "Здесь отображаются подтверждённые статусы служб и интеграций. Непроверенные компоненты не считаются подключёнными."
                            }
                            Text { text: "Статус: рабочие состояния подключаются по мере реализации"; color: root.amber; font.pixelSize: 10; wrapMode: Text.Wrap }
                        }
                    }
                }
            }
        }
    }
}
