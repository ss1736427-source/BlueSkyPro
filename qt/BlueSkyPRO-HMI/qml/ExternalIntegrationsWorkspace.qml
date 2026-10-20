import QtQuick
import QtQuick.Controls

Item {
    id: root
    property bool showTechnicalConsole: false
    readonly property color bg: "#07111B"
    readonly property color card: "#152431"
    readonly property color line: "#2B3D4D"
    readonly property color fg: "#E8EEF5"
    readonly property color muted: "#91A4B7"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#F5BE42"
    readonly property color green: "#36D6A0"

    Rectangle { anchors.fill: parent; color: root.bg }
    Loader {
        anchors.fill: parent
        active: root.showTechnicalConsole
        sourceComponent: Component {
            Item {
                anchors.fill: parent
                IntegrationConsole { anchors.fill: parent }
                Rectangle {
                    z: 5; anchors.left: parent.left; anchors.top: parent.top; anchors.margins: 14
                    width: backLabel.implicitWidth + 26; height: 34; radius: 5
                    color: "#17394E"; border.color: root.line
                    Text { id: backLabel; anchors.centerIn: parent; text: "←  К интеграциям"; color: root.fg; font.pixelSize: 11 }
                    MouseArea { anchors.fill: parent; onClicked: root.showTechnicalConsole = false }
                }
            }
        }
    }
    Flickable {
        anchors.fill: parent
        visible: !root.showTechnicalConsole
        clip: true
        contentWidth: width
        contentHeight: page.implicitHeight + 48
        Column {
            id: page
            x: 30; y: 26
            width: Math.max(320, parent.width - 60)
            spacing: 18
            Text { text: "Внешние интеграции"; color: root.fg; font.pixelSize: 26; font.bold: true }
            Text {
                width: parent.width; wrapMode: Text.Wrap
                text: "Подключайте источники по одному. Сначала выберите направление — подробные поля появятся только на следующем шаге."
                color: root.muted; font.pixelSize: 13
            }
            Grid {
                width: parent.width
                columns: width < 700 ? 1 : 2
                spacing: 14
                Repeater {
                    model: [
                        {title:"NOTAM / AIP",desc:"Аэронавигационные ограничения и извещения.",status:"Источник РФ не подключён",kind:"notam",state:2},
                        {title:"Погода / WX",desc:"Погода и прогноз для оценки условий полёта.",status:"Open-Meteo · тестовый запрос",kind:"weather",state:1},
                        {title:"Карты и GIS",desc:"Картографическая подложка и стили карты.",status:"MapTiler · ключ настраивается отдельно",kind:"maps",state:1},
                        {title:"Рельеф / DEM",desc:"Точечная высота местности; профиль рельефа отдельно не подключён.",status:"Open-Meteo Elevation · тестовый запрос",kind:"terrain",state:1},
                        {title:"Воздушное пространство",desc:"Зоны, ограничения и геозоны.",status:"Адаптер не подключён",kind:"airspace",state:0},
                        {title:"Препятствия",desc:"Источники препятствий и покрытие.",status:"Источник не подключён",kind:"obstacles",state:0},
                        {title:"БПЛА и каналы связи",desc:"Автопилот, телеметрия, C2 и поддерживаемые протоколы.",status:"Единый адаптерный каталог не реализован",kind:"uav",state:0},
                        {title:"GNSS / RTK / PPK",desc:"Навигационные поправки и связанные сервисы.",status:"Адаптер не подключён",kind:"gnss",state:0},
                        {title:"Страхование",desc:"Проверка страхового покрытия и обмен данными.",status:"Провайдер не подключён",kind:"insurance",state:0},
                        {title:"Другие источники",desc:"Трафик/UTM, площадки, ATM, видео, журналы и внешние API.",status:"Настраиваются после определения интерфейсов",kind:"other",state:0}
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        width: (page.width - (page.width < 700 ? 0 : 14)) / (page.width < 700 ? 1 : 2)
                        height: 142; radius: 8; color: root.card; border.color: root.line
                        Column {
                            anchors.fill: parent; anchors.margins: 16; spacing: 8
                            Row {
                                width: parent.width; spacing: 8
                                Rectangle { width: 7; height: 7; radius: 4; anchors.verticalCenter: parent.verticalCenter; color: modelData.state === 0 ? root.muted : modelData.state === 1 ? root.amber : "#F28B82" }
                                Text { text: modelData.title; color: root.fg; font.pixelSize: 15; font.bold: true; width: parent.width - 20; wrapMode: Text.Wrap }
                            }
                            Text { text: modelData.desc; color: root.muted; font.pixelSize: 11; wrapMode: Text.Wrap; width: parent.width; maximumLineCount: 2 }
                            Item { width: 1; height: 1 }
                            Text { text: modelData.status; color: modelData.state === 0 ? root.muted : root.amber; font.pixelSize: 10; wrapMode: Text.Wrap; width: parent.width }
                            Text {
                                text: modelData.state === 0 ? "Требуется настройка интерфейса" : "Открыть настройки и проверки  →"
                                color: modelData.state === 0 ? root.muted : root.cyan
                                font.pixelSize: 10
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            enabled: modelData.state !== 0
                            onClicked: root.showTechnicalConsole = true
                        }
                    }
                }
            }
            Rectangle {
                width: parent.width; height: note.implicitHeight + 28; radius: 7
                color: "#101D28"; border.color: root.line
                Column { id: note; anchors.fill: parent; anchors.margins: 14; spacing: 7
                    Text { text: "ВАЖНО"; color: root.cyan; font.pixelSize: 10; font.bold: true }
                    Text {
                        width: parent.width; wrapMode: Text.Wrap
                        text: "Статусы отражают текущее состояние прототипа. Доступность URL сама по себе не означает, что данные проверены и пригодны для планирования. Российский NOTAM-источник пока не подключён; бесплатные тестовые источники не заменяют эксплуатационные данные."
                        color: root.muted; font.pixelSize: 11
                    }
                }
            }
        }
    }
}