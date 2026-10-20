import QtQuick
import QtQuick.Controls

Item {
    id: root
    property string activeSection: "Overview"
    readonly property color bg: "#07111B"
    readonly property color sidebar: "#101D28"
    readonly property color card: "#152431"
    readonly property color line: "#2B3D4D"
    readonly property color fg: "#E8EEF5"
    readonly property color muted: "#91A4B7"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#F5BE42"

    function titleFor(key) { return ({Overview:"Обзор системы",Users:"Пользователи",Roles:"Роли и доступ",Security:"Безопасность и вход",Settings:"Настройки системы",Integrations:"Внешние интеграции",Sync:"Данные и синхронизация",Documents:"Документы и соответствие",Audit:"Журнал аудита",Diagnostics:"Обслуживание и диагностика"})[key] || "Администрирование" }
    function descriptionFor(key) { return ({Overview:"Общий обзор состояния системы, служб, подключений и критических сообщений.",Users:"Индивидуальные учётные записи, блокировка пользователей и процедура сброса доступа.",Roles:"Назначение ролей и разрешений по принципу минимально необходимых полномочий.",Security:"Политика входа, защита сессий и события безопасности.",Settings:"Общие параметры приложения и эксплуатационные политики.",Integrations:"Подключение NOTAM, WX, картографии, рельефа и других внешних источников.",Sync:"Импорт, экспорт, синхронизация и ошибки обмена данными.",Documents:"Контролируемые документы, требования и подтверждения проверок.",Audit:"История входов, отказов доступа и административных изменений.",Diagnostics:"Диагностика служб и отчёты; секреты должны быть скрыты."})[key] || "" }
    Rectangle { anchors.fill: parent; color: root.bg }
    Row { anchors.fill: parent; spacing: 0
        Rectangle { id: navigation; width: 248; color: root.sidebar; border.color: root.line; border.width: 1
            Column { anchors.fill: parent; spacing: 0
                Rectangle { width: parent.width; height: 78; color: "#0B1722"
                    Column { anchors.left: parent.left; anchors.leftMargin: 17; anchors.verticalCenter: parent.verticalCenter; spacing: 3
                        Text { text: "◢  BlueSky PRO"; color: root.cyan; font.pixelSize: 19; font.bold: true }
                        Text { text: "ADMINISTRATOR"; color: root.muted; font.pixelSize: 9; font.letterSpacing: 1.2 }
                    }
                }
                Flickable { width: parent.width; height: parent.height - 118; clip: true; contentWidth: width; contentHeight: navItems.implicitHeight
                    Column { id: navItems; width: parent.width; spacing: 0
                        Repeater { model: [
                            {key:"Overview",title:"Обзор системы",glyph:"⌂",group:"СИСТЕМА"},
                            {key:"Users",title:"Пользователи",glyph:"♙",group:""},
                            {key:"Roles",title:"Роли и доступ",glyph:"⚿",group:""},
                            {key:"Security",title:"Безопасность и вход",glyph:"◈",group:""},
                            {key:"Settings",title:"Настройки системы",glyph:"⚙",group:""},
                            {key:"Integrations",title:"Внешние интеграции",glyph:"◎",group:"ПОДКЛЮЧЕНИЯ И ДАННЫЕ"},
                            {key:"Sync",title:"Данные и синхронизация",glyph:"↻",group:""},
                            {key:"Documents",title:"Документы и соответствие",glyph:"▤",group:"КОНТРОЛЬ И ОБСЛУЖИВАНИЕ"},
                            {key:"Audit",title:"Журнал аудита",glyph:"▣",group:""},
                            {key:"Diagnostics",title:"Обслуживание и диагностика",glyph:"⌁",group:""}
                        ];
                        delegate: Column { required property var modelData; width: navItems.width; spacing: 0
                            Rectangle { visible: modelData.group !== ""; width: parent.width; height: visible ? 27 : 0; color: "transparent"
                                Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.bottom: parent.bottom; anchors.bottomMargin: 5; text: modelData.group; color: "#71869A"; font.pixelSize: 8; font.bold: true; font.letterSpacing: 1 }
                            }
                            Rectangle { width: parent.width; height: 48; color: root.activeSection === modelData.key ? "#17394E" : "transparent"
                                Rectangle { width: 4; height: parent.height; color: root.activeSection === modelData.key ? root.cyan : "transparent" }
                                Row { anchors.fill: parent; anchors.leftMargin: 13; spacing: 10
                                    Text { width: 22; anchors.verticalCenter: parent.verticalCenter; text: modelData.glyph; color: root.activeSection === modelData.key ? root.cyan : root.muted; font.pixelSize: 18; horizontalAlignment: Text.AlignHCenter }
                                    Text { width: parent.width - 48; anchors.verticalCenter: parent.verticalCenter; text: modelData.title; color: root.activeSection === modelData.key ? root.fg : "#C3CFDA"; font.pixelSize: 12; elide: Text.ElideRight }
                                }
                                MouseArea { anchors.fill: parent; onClicked: root.activeSection = modelData.key }
                            }
                        } }
                    }
                }
                Rectangle { width: parent.width; height: 40; color: "#0B1722"; Text { anchors.left: parent.left; anchors.leftMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "‹‹  Административная панель"; color: root.muted; font.pixelSize: 10 } }
            }
        }
        Item { id: contentArea; width: Math.max(0, root.width - navigation.width); height: root.height
            // Compact administrator top bar; flight header and toolbar are hidden in ADMIN mode.
            Rectangle {
                id: adminTopBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 52
                color: "#0B1722"
                border.color: root.line
                border.width: 1
                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 22
                    anchors.rightMargin: 24
                    spacing: 12
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "⌂"; color: root.cyan; font.pixelSize: 22 }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "Администратор"; color: root.muted; font.pixelSize: 11 }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "›"; color: root.muted; font.pixelSize: 18 }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: root.titleFor(root.activeSection); color: root.fg; font.pixelSize: 12; font.bold: true }
                    Item { width: Math.max(10, parent.width - 530); height: 1 }
                    Text { anchors.verticalCenter: parent.verticalCenter; text: "ADMINISTRATOR"; color: root.muted; font.pixelSize: 9; font.letterSpacing: 1 }
                }
            }
            Loader { anchors.fill: parent; anchors.topMargin: adminTopBar.height; active: root.activeSection === "Integrations"; visible: active; sourceComponent: Component { ExternalIntegrationsWorkspace { anchors.fill: parent } } }
            Flickable { anchors.fill: parent; anchors.topMargin: adminTopBar.height; visible: root.activeSection !== "Integrations"; clip: true; contentWidth: width; contentHeight: page.implicitHeight + 52
                Column { id: page; x: 28; y: 26; width: Math.max(300, parent.width - 56); spacing: 20
                    Text { text: root.titleFor(root.activeSection); color: root.fg; font.pixelSize: 25; font.bold: true }
                    Text { width: parent.width; wrapMode: Text.Wrap; text: root.descriptionFor(root.activeSection); color: root.muted; font.pixelSize: 13 }
                    Grid { width: parent.width; columns: width < 760 ? 1 : 3; spacing: 12
                        Repeater { model: [{title:"Внешние интеграции",desc:"NOTAM · WX · карты · рельеф",target:"Integrations"},{title:"Пользователи и роли",desc:"Учётные записи и разрешения",target:"Users"},{title:"Журнал аудита",desc:"Входы, изменения и события",target:"Audit"}]
                            delegate: Rectangle { required property var modelData; width: (page.width - (page.width < 760 ? 0 : 24)) / (page.width < 760 ? 1 : 3); height: 126; radius: 8; color: root.card; border.color: root.line
                                Column {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 10
                                    Text { text: modelData.title; color: root.fg; font.pixelSize: 15; font.bold: true }
                                    Text { text: modelData.desc; color: root.muted; font.pixelSize: 12; wrapMode: Text.Wrap; width: parent.width }
                                    Text { text: "Открыть  →"; color: root.cyan; font.pixelSize: 11 }
                                }
                                MouseArea { anchors.fill: parent; onClicked: root.activeSection = modelData.target }
                            }
                        }
                    }
                    Rectangle { width: parent.width; height: detail.implicitHeight + 32; radius: 7; color: root.card; border.color: root.line
                        Column { id: detail; anchors.fill: parent; anchors.margins: 16; spacing: 10; Text { text: root.activeSection === "Overview" ? "СОСТОЯНИЕ СИСТЕМЫ" : "РАБОЧАЯ ОБЛАСТЬ"; color: root.cyan; font.pixelSize: 10; font.bold: true }
                            Text { width: parent.width; wrapMode: Text.Wrap; color: root.fg; font.pixelSize: 12; text: root.activeSection === "Overview" ? "Отображаются фактические статусы служб, подключений и ошибок. Непроверенные компоненты не показываются как работающие." : root.activeSection === "Users" ? "Интерфейсный каркас. Создание пользователей включается после реализации защищённой аутентификации и модели разрешений." : root.activeSection === "Roles" ? "Роли и разрешения должны проверяться прикладным ядром, а не только видимостью элементов интерфейса." : root.activeSection === "Security" ? "Настройки политики входа и сессий. Пароли не хранятся в открытом виде; секреты не выводятся в журналы." : root.activeSection === "Settings" ? "Общие параметры приложения. Секреты интеграций настраиваются отдельно в защищённом хранилище." : root.activeSection === "Sync" ? "Статусы обмена данными появятся после подключения служб и проверки результатов." : root.activeSection === "Documents" ? "Контролируемые документы, требования, доказательства верификации и записи выпуска." : root.activeSection === "Audit" ? "История входов, отказов доступа и изменений настроек, ролей и интеграций." : "Диагностические данные должны скрывать ключи, токены, пароли и другие секреты." }
                            Text { text: "Статус: интерфейсный каркас; рабочие статусы подключаются по мере реализации"; color: root.amber; font.pixelSize: 10; wrapMode: Text.Wrap }
                        }
                    }
                }
            }
        }
    }
}