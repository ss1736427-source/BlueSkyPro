import QtQuick
import QtQuick.Controls

Item {
    id: root
    property string selectedKind: ""
    property int wizardStep: 0
    property string endpointText: "https://api-staging.cgifederal-aim.com/nmsapi/v1"
    property string faaClientIdText: ""
    property string faaClientSecretText: ""
    property bool faaNmsMode: false
    property bool faaCredentialsSaved: false
    property string keyText: ""
    property string endpointCheck: ""
    property string wizardMessage: ""
    property string wizardState: "idle"
    property bool notamSourceConfigured: false
    property string notamSourceStatus: "Требуется настройка источника"
    property bool showTechnicalConsole: false
    readonly property color bg: "#07111B"
    readonly property color card: "#152431"
    readonly property color line: "#2B3D4D"
    readonly property color fg: "#E8EEF5"
    readonly property color muted: "#91A4B7"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#F5BE42"
    readonly property color green: "#36D6A0"
    readonly property color red: "#F28B82"

    function refreshFaaCredentialsSaved() {
        faaCredentialsSaved = legacyConsole && legacyConsole.credentialVaultAvailable()
                && credentialVault.hasSecret("faa_nms_client_id")
                && credentialVault.hasSecret("faa_nms_client_secret")
    }

    function chooseSource(kind) {
        selectedKind = kind
        wizardStep = 0
        wizardMessage = ""
        wizardState = "idle"
        if (kind !== "notam") showTechnicalConsole = true
        faaNmsMode = false
        faaClientIdText = ""
        faaClientSecretText = ""
        refreshFaaCredentialsSaved()
    }
    function validateEndpoint() {
        var value = endpointText.trim()
        if (!(value.indexOf("https://") === 0 || value.indexOf("http://") === 0)) {
            wizardState = "error"
            wizardMessage = "Адрес должен начинаться с https:// или http://."
            return
        }
        if (value.indexOf(" ") >= 0 || value.length < 12) {
            wizardState = "error"
            wizardMessage = "Проверьте адрес: он выглядит неполным или содержит пробелы."
            return
        }
        endpointText = value.replace(/\/+$/, "")
        endpointCheck = endpointText
        faaNmsMode = endpointText.indexOf("cgifederal-aim.com") >= 0
                || endpointText.indexOf("api-nms.aim.faa.gov") >= 0
        refreshFaaCredentialsSaved()
        wizardState = "accepted"
        wizardMessage = "Формат адреса принят. Это ещё не подтверждает доступность источника."
        wizardStep = 1
    }
    function runNotamTest() {
        if (!legacyConsole) return
        wizardStep = 2
        wizardState = "testing"
        wizardMessage = ""
        if (faaNmsMode) {
            if (endpointText.indexOf("api-staging.cgifederal-aim.com") >= 0) {
                faaNmsClient.environment = "STAGING"
            } else if (endpointText.indexOf("api-fit.cgifederal-aim.com") >= 0) {
                faaNmsClient.environment = "FIT"
            } else {
                wizardState = "error"
                wizardMessage = "Этот адрес FAA NMS не включён в тестовый клиент. Используйте выданный FAA адрес STAGING или FIT; Production не проверяется этим мастером."
                wizardStep = 1
                return
            }
            if (faaClientIdText.trim().length > 0 || faaClientSecretText.length > 0) {
                legacyConsole.faaClientIdInput = faaClientIdText
                legacyConsole.faaClientSecretInput = faaClientSecretText
                legacyConsole.saveFaaNmsCredentials()
                faaClientIdText = ""
                faaClientSecretText = ""
                refreshFaaCredentialsSaved()
                if (!faaCredentialsSaved || legacyConsole.faaNmsMessage.indexOf("saved separately") < 0) {
                    wizardState = "error"
                    wizardMessage = legacyConsole.faaNmsMessage
                    wizardStep = 1
                    return
                }
            } else {
                refreshFaaCredentialsSaved()
                if (!faaCredentialsSaved) {
                wizardState = "error"
                wizardMessage = "Введите Client ID и Client Secret. Оба значения обязательны."
                wizardStep = 1
                return
                }
            }
            wizardMessage = "Выполняется OAuth-аутентификация FAA NMS и проверка ping…"
            faaNmsClient.testConnection()
            wizardState = "result"
            return
        }
        legacyConsole.notamBaseUrl = endpointText
        if (keyText.trim().length > 0) {
            legacyConsole.notamApiKey = keyText
            legacyConsole.saveProviderSecret("notam")
            keyText = ""
        } else if (!legacyConsole.credentialVaultAvailable()
                   || !credentialVault.hasSecret("notam")) {
            wizardState = "error"
            wizardMessage = "Для проверки NOTAC нужен токен. Введите ключ или сначала сохраните его в системном хранилище."
            wizardStep = 1
            return
        } else {
            legacyConsole.notamApiKey = ""
        }
        wizardMessage = "Проверяем авторизацию и ответ NOTAC API…"
        legacyConsole.requestNotamApiTest()
        wizardState = "result"
    }

    Rectangle { anchors.fill: parent; color: root.bg }
    IntegrationConsole { id: legacyConsole; visible: false; width: 1; height: 1 }
    Connections {
        target: typeof faaNmsClient !== "undefined" ? faaNmsClient : null
        function onConnectionTestFinished(success, message) {
            if (root.faaNmsMode) {
                root.notamSourceConfigured = success
                root.notamSourceStatus = success
                    ? "Источник настроен · FAA NMS STAGING · соединение проверено"
                    : "Ошибка подключения · требуется проверка"
                root.wizardMessage = message
                root.wizardState = success ? "success" : "error"
            }
        }
    }

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
                    MouseArea { anchors.fill: parent; onClicked: { root.showTechnicalConsole = false; root.selectedKind = "" } }
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
            Item {
                width: parent.width
                height: Math.max(44, heading.implicitHeight)
                Text {
                    id: heading
                    anchors.left: parent.left
                    anchors.right: addSourceButton.visible ? addSourceButton.left : parent.right
                    anchors.rightMargin: addSourceButton.visible ? 18 : 0
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.selectedKind === "notam" ? "Подключение NOTAM / AIP" : "Внешние интеграции"
                    color: root.fg
                    font.pixelSize: 28
                    font.bold: true
                    wrapMode: Text.Wrap
                }
                Rectangle {
                    id: addSourceButton
                    visible: root.selectedKind === ""
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 174
                    height: 42
                    radius: 6
                    color: root.cyan
                    Text { anchors.centerIn: parent; text: "+  Добавить источник"; color: "#07111B"; font.pixelSize: 12; font.bold: true }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.chooseSource("notam") }
                }
            }
            Text {
                width: parent.width; wrapMode: Text.Wrap
                text: root.selectedKind === "notam" ? "Настройка по шагам. Адрес принят — далее система запросит ключ и проверит чтение данных." : "Выберите направление. Кнопка «Добавить источник» открывает доступный мастер NOTAM; остальные адаптеры подключаются по мере реализации."
                color: root.muted; font.pixelSize: 13
            }

            Rectangle {
                visible: root.selectedKind === "notam"
                width: parent.width; height: wizardCard.implicitHeight + 36
                radius: 9; color: root.card; border.color: root.line
                Column {
                    id: wizardCard; anchors.fill: parent; anchors.margins: 20; spacing: 16
                    Row { spacing: 8
                        Repeater { model: ["Адрес","Ключ","Проверка"]
                            delegate: Row { required property string modelData; required property int index; spacing: 6
                                Rectangle { width: 22; height: 22; radius: 11; color: root.wizardStep >= index ? root.cyan : "#26394B"
                                    Text { anchors.centerIn: parent; text: String(index + 1); color: root.wizardStep >= index ? "#07111B" : root.muted; font.pixelSize: 10; font.bold: true } }
                                Text { anchors.verticalCenter: parent.verticalCenter; text: modelData; color: root.wizardStep === index ? root.fg : root.muted; font.pixelSize: 11 }
                            }
                        }
                    }
                    Text { text: root.wizardStep === 0 ? "Шаг 1. Укажите адрес источника" : root.wizardStep === 1 ? "Шаг 2. Ключ доступа" : "Шаг 3. Проверка источника"; color: root.fg; font.pixelSize: 18; font.bold: true }
                    Text {
                        width: parent.width; wrapMode: Text.Wrap; color: root.muted; font.pixelSize: 12
                        text: root.wizardStep === 0 ? "Укажите базовый URL выбранного источника. Для FAA NMS используйте выданный адрес STAGING или FIT." : root.wizardStep === 1 ? (root.faaNmsMode ? (root.faaCredentialsSaved ? "Client ID и Client Secret уже сохранены в Windows Credential Manager. Оставьте оба поля пустыми, чтобы использовать сохранённые данные; введите оба значения только для замены." : "FAA NMS использует OAuth: введите Client ID и Client Secret. После сохранения они останутся в защищённом хранилище Windows и не потребуют повторного ввода.") : "Для NOTAC введите API-ключ/токен. Он сохраняется в системном защищённом хранилище.") : (root.faaNmsMode ? "Проверяется OAuth-аутентификация FAA NMS и доступность ping. Успешный ping ещё не означает, что NOTAM загружены." : "Тест проверяет ответ NOTAC REST API по ожидаемой JSON-схеме; он не подтверждает покрытие воздушного пространства РФ.")
                    }
                    TextField {
                        visible: root.wizardStep === 0
                        width: parent.width
                        text: root.endpointText
                        onTextChanged: root.endpointText = text
                        placeholderText: "https://provider.example/api"
                        color: root.fg; placeholderTextColor: root.muted
                        background: Rectangle { color: "#0B1724"; border.color: root.line; radius: 5 }
                    }
                    Text {
                        visible: root.wizardStep === 1 && root.faaNmsMode && root.faaCredentialsSaved
                        width: parent.width; wrapMode: Text.Wrap
                        text: "Сохранённые учётные данные найдены. Повторно вводить их не нужно."
                        color: root.green; font.pixelSize: 11
                    }
                    TextField {
                        visible: root.wizardStep === 1 && root.faaNmsMode
                        width: parent.width
                        text: root.faaClientIdText
                        onTextChanged: root.faaClientIdText = text
                        placeholderText: "FAA NMS Client ID"
                        color: root.fg; placeholderTextColor: root.muted
                        background: Rectangle { color: "#0B1724"; border.color: root.line; radius: 5 }
                    }
                    TextField {
                        visible: root.wizardStep === 1 && root.faaNmsMode
                        width: parent.width
                        text: root.faaClientSecretText
                        onTextChanged: root.faaClientSecretText = text
                        placeholderText: "FAA NMS Client Secret"
                        echoMode: TextInput.Password
                        color: root.fg; placeholderTextColor: root.muted
                        background: Rectangle { color: "#0B1724"; border.color: root.line; radius: 5 }
                    }
                    TextField {
                        visible: root.wizardStep === 1 && !root.faaNmsMode
                        width: parent.width
                        text: root.keyText
                        onTextChanged: root.keyText = text
                        placeholderText: "Ключ / токен (если требуется)"
                        echoMode: TextInput.Password
                        color: root.fg; placeholderTextColor: root.muted
                        background: Rectangle { color: "#0B1724"; border.color: root.line; radius: 5 }
                    }
                    Text {
                        visible: root.wizardStep === 2 || root.wizardMessage.length > 0
                        width: parent.width; wrapMode: Text.Wrap
                        text: root.wizardStep === 2 && root.faaNmsMode
                            ? "Результат теста FAA NMS: " + legacyConsole.faaNmsMessage
                            : root.wizardStep === 2 && root.selectedKind === "notam"
                                ? "Результат теста NOTAC: " + legacyConsole.notamApiStatus
                                    + (legacyConsole.notamApiError.length ? "\n" + legacyConsole.notamApiError : "")
                                    + (legacyConsole.notamApiSample.length ? "\nПример данных: " + legacyConsole.notamApiSample : "")
                                : root.wizardMessage
                        color: root.wizardState === "error" ? root.red
                            : root.wizardStep === 2 && legacyConsole.notamApiStatus === "CONNECTED / JSON VALID" ? root.green
                             : root.wizardStep === 2 && root.faaNmsMode && legacyConsole.faaNmsMessage.indexOf("succeeded") >= 0 ? root.green
                            : root.wizardStep === 2 ? root.amber : root.cyan
                        font.pixelSize: 11
                    }
                    Row { spacing: 10
                        Rectangle { visible: root.wizardStep > 0 && root.wizardStep < 2; width: 100; height: 38; radius: 5; color: "#223442"
                            Text { anchors.centerIn: parent; text: "Назад"; color: root.fg; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: { root.wizardStep -= 1; root.wizardState = "idle"; root.wizardMessage = "" } }
                        }
                        Rectangle { visible: root.wizardStep === 0; width: 170; height: 38; radius: 5; color: root.cyan
                            Text { anchors.centerIn: parent; text: "Принять адрес  →"; color: "#07111B"; font.pixelSize: 12; font.bold: true }
                            MouseArea { anchors.fill: parent; onClicked: root.validateEndpoint() }
                        }
                        Rectangle { visible: root.wizardStep === 1; width: 190; height: 38; radius: 5; color: root.cyan
                            Text { anchors.centerIn: parent; text: "Сохранить и проверить  →"; color: "#07111B"; font.pixelSize: 12; font.bold: true }
                            MouseArea { anchors.fill: parent; onClicked: root.runNotamTest() }
                        }
                        Rectangle { visible: root.wizardStep === 2; width: 130; height: 38; radius: 5; color: "#223442"
                            Text { anchors.centerIn: parent; text: "Повторить тест"; color: root.fg; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: root.runNotamTest() }
                        }
                        Rectangle { visible: root.wizardStep === 2; width: 100; height: 38; radius: 5; color: root.cyan
                            Text { anchors.centerIn: parent; text: "Готово"; color: "#07111B"; font.pixelSize: 12; font.bold: true }
                            MouseArea { anchors.fill: parent; onClicked: { root.selectedKind = ""; root.wizardStep = 0; root.wizardMessage = ""; root.wizardState = "idle" } }
                        }
                    }
                }
            }

            Grid {
                id: integrationsGrid
                visible: root.selectedKind === ""
                width: parent.width
                columns: width < 700 ? 1 : width < 1120 ? 2 : 3
                spacing: 14
                Repeater {
                    model: [
                        {title:"NOTAM / AIP",desc:"Аэронавигационные данные и ограничения.",status:root.notamSourceStatus,kind:"notam",state:root.notamSourceConfigured ? 1 : 2},
                        {title:"Погода (WX)",desc:"Метеоданные, прогнозы, METAR / TAF.",status:"Open-Meteo · тестовый источник",kind:"weather",state:1},
                        {title:"Карты и GIS",desc:"Картографические сервисы и спутниковые снимки.",status:"MapTiler · ключ настраивается отдельно",kind:"maps",state:1},
                        {title:"Рельеф и препятствия",desc:"Цифровая модель рельефа, высоты и препятствия.",status:"Рельеф: тестовый запрос · препятствия: не настроены",kind:"terrain",state:1},
                        {title:"БПЛА и автопилоты",desc:"MAVLink, телеметрия и управление.",status:"Адаптерный каталог в разработке",kind:"uav",state:0},
                        {title:"Связь и C2",desc:"Каналы связи, телеметрия и спутниковые системы.",status:"Не настроено",kind:"c2",state:0},
                        {title:"Страхование",desc:"Страховые сервисы и проверка покрытия.",status:"Провайдер не подключён",kind:"insurance",state:0},
                        {title:"Прочие интеграции",desc:"UTM, ATM, внешние API и другие сервисы.",status:"Требуется определить интерфейсы",kind:"other",state:0}
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        width: (integrationsGrid.width - integrationsGrid.spacing * (integrationsGrid.columns - 1)) / integrationsGrid.columns
                        height: 158
                        radius: 9
                        color: root.card
                        border.color: root.line
                        Row {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 14
                            Rectangle {
                                width: 64; height: 64; radius: 9
                                color: "#1C3447"
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.kind === "notam" ? "➤"
                                        : modelData.kind === "weather" ? "☁"
                                        : modelData.kind === "maps" ? "⌖"
                                        : modelData.kind === "terrain" ? "▲"
                                        : modelData.kind === "uav" ? "⌘"
                                        : modelData.kind === "c2" ? "⌁"
                                        : modelData.kind === "insurance" ? "◆" : "▤"
                                    color: "#35BDF5"
                                    font.pixelSize: 30
                                }
                            }
                            Column {
                                width: Math.max(0, parent.width - 82)
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 8
                                Text {
                                    text: modelData.title
                                    color: root.fg
                                    font.pixelSize: 15
                                    font.bold: true
                                    width: parent.width
                                    wrapMode: Text.Wrap
                                }
                                Text { text: modelData.desc; color: root.muted; font.pixelSize: 12; wrapMode: Text.Wrap; width: parent.width; maximumLineCount: 2 }
                                Row {
                                    spacing: 6
                                    Rectangle {
                                        width: 9; height: 9; radius: 5; anchors.verticalCenter: parent.verticalCenter
                                        color: modelData.kind === "notam" && root.notamSourceConfigured ? root.green : (modelData.state === 0 ? "#AFC4D9" : root.amber)
                                    }
                                    Text { text: modelData.status; color: modelData.kind === "notam" && root.notamSourceConfigured ? root.green : (modelData.state === 0 ? root.muted : root.amber); font.pixelSize: 11; wrapMode: Text.Wrap; width: Math.max(0, parent.width - 16) }
                                }
                            }
                        }
                        MouseArea { anchors.fill: parent; enabled: modelData.state !== 0; onClicked: root.chooseSource(modelData.kind) }
                    }
                }
            }
            Rectangle {
                visible: root.selectedKind === ""
                width: parent.width; height: note.implicitHeight + 28; radius: 7; color: "#101D28"; border.color: root.line
                Column { id: note; anchors.fill: parent; anchors.margins: 14; spacing: 7
                    Text { text: "ВАЖНО"; color: root.cyan; font.pixelSize: 10; font.bold: true }
                    Text { width: parent.width; wrapMode: Text.Wrap
                        text: "Статус соединения не равен готовности данных к планированию. Российский NOTAM-источник пока не подключён; бесплатные тестовые источники не заменяют эксплуатационные данные."
                        color: root.muted; font.pixelSize: 11
                    }
                }
            }
        }
    }
}