import QtQuick
import QtQuick.Controls

Item {
    id: root
    property int selectedIntegration: 0
    property string latitudeText: ""
    property string longitudeText: ""
    property string wxStatus: "NOT TESTED"
    property string demStatus: "NOT TESTED"
    property string notamStatus: "ACCESS NOT CONFIGURED"
    property string notamBaseUrl: "https://notac.aero/api/v1"
    property string notamApiKey: ""
    property string notamApiStatus: "NOT CONFIGURED"
    property string notamApiError: ""
    property int notamApiCount: -1
    property string notamApiSample: ""
    property string icaoApiKey: ""
    property string mapTilerApiKey: ""
    property string wxCommercialApiKey: ""
    property bool notamSecretSaved: false
    property bool icaoSecretSaved: false
    property bool mapTilerSecretSaved: false
    property bool wxSecretSaved: false
    property string credentialStoreStatus: "CHECKING"
    property string credentialStoreMessage: ""
    property var wxResult: null
    property var demResult: null
    property string wxError: ""
    property string demError: ""
    readonly property color panel: "#0B1724"
    readonly property color line: "#26394B"
    readonly property color text: "#E8EEF5"
    readonly property color muted: "#8EA2B8"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#FFD166"
    readonly property color green: "#38C172"
    readonly property color red: "#E74C3C"


    function credentialVaultAvailable() { return typeof credentialVault !== "undefined" && credentialVault !== null }
    function refreshCredentialStates() {
        if (!credentialVaultAvailable()) {
            notamSecretSaved = false; icaoSecretSaved = false; mapTilerSecretSaved = false; wxSecretSaved = false
            credentialStoreStatus = "NATIVE VAULT UNAVAILABLE"; return
        }
        notamSecretSaved = credentialVault.hasSecret("notam")
        icaoSecretSaved = credentialVault.hasSecret("icao")
        mapTilerSecretSaved = credentialVault.hasSecret("maptiler")
        wxSecretSaved = credentialVault.hasSecret("weather")
        credentialStoreStatus = credentialVault.backendStatus()
    }
    function providerSecret(provider, enteredValue) {
        var candidate = String(enteredValue || "").trim()
        if (candidate.length > 0) return candidate
        if (credentialVaultAvailable() && credentialVault.hasSecret(provider)) return credentialVault.readSecret(provider)
        return ""
    }
    function saveProviderSecret(provider) {
        if (!credentialVaultAvailable()) { credentialStoreMessage = "Native credential vault unavailable. No key saved."; return }
        var candidate = ""
        if (provider === "notam") candidate = notamApiKey
        else if (provider === "icao") candidate = icaoApiKey
        else if (provider === "maptiler") candidate = mapTilerApiKey
        else if (provider === "weather") candidate = wxCommercialApiKey
        if (String(candidate || "").trim().length === 0) { credentialStoreMessage = "Enter a key before saving."; return }
        if (!credentialVault.saveSecret(provider, candidate)) { credentialStoreMessage = "SAVE FAILED: " + credentialVault.lastError; return }
        if (provider === "notam") notamApiKey = ""
        else if (provider === "icao") icaoApiKey = ""
        else if (provider === "maptiler") mapTilerApiKey = ""
        else if (provider === "weather") wxCommercialApiKey = ""
        credentialStoreMessage = provider.toUpperCase() + " key saved to " + credentialVault.backendStatus() + "."
        refreshCredentialStates()
    }
    function deleteProviderSecret(provider) {
        if (!credentialVaultAvailable()) { credentialStoreMessage = "Native credential vault unavailable. No key deleted."; return }
        if (!credentialVault.deleteSecret(provider)) { credentialStoreMessage = "DELETE FAILED: " + credentialVault.lastError; return }
        credentialStoreMessage = provider.toUpperCase() + " saved key deleted."
        refreshCredentialStates()
    }

    function coordinatesValid() {
        var lat = Number(latitudeText)
        var lon = Number(longitudeText)
        return latitudeText.trim() !== "" && longitudeText.trim() !== ""
                && isFinite(lat) && isFinite(lon)
                && lat >= -90 && lat <= 90 && lon >= -180 && lon <= 180
    }

    function requestWeather() {
        if (!coordinatesValid()) {
            wxStatus = "INVALID COORDINATES"
            wxError = "Enter latitude (-90…90) and longitude (-180…180)."
            wxResult = null
            return
        }
        wxStatus = "CONNECTING"
        wxError = ""
        wxResult = null
        var lat = Number(latitudeText)
        var lon = Number(longitudeText)
        var url = "https://api.open-meteo.com/v1/forecast?latitude=" + lat
                + "&longitude=" + lon
                + "&current=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,weather_code,cloud_cover,pressure_msl,wind_speed_10m,wind_direction_10m,wind_gusts_10m"
                + "&hourly=visibility,cloud_base"
                + "&forecast_days=1&timezone=UTC"
        var request = new XMLHttpRequest()
        request.onreadystatechange = function() {
            if (request.readyState !== XMLHttpRequest.DONE)
                return
            if (request.status >= 200 && request.status < 300) {
                try {
                    var parsed = JSON.parse(request.responseText)
                    if (!parsed.current || !parsed.current.time)
                        throw new Error("Response has no current observation/model snapshot")
                    root.wxResult = parsed
                    root.wxStatus = "CONNECTED / DATA RECEIVED"
                    root.wxError = ""
                } catch (error) {
                    root.wxStatus = "INVALID RESPONSE"
                    root.wxError = String(error)
                }
            } else {
                root.wxStatus = "CONNECTION FAILED"
                root.wxError = "HTTP " + request.status + ": " + request.statusText
            }
        }
        request.open("GET", url)
        request.send()
    }

    function requestElevation() {
        if (!coordinatesValid()) {
            demStatus = "INVALID COORDINATES"
            demError = "Enter latitude (-90…90) and longitude (-180…180)."
            demResult = null
            return
        }
        demStatus = "CONNECTING"
        demError = ""
        demResult = null
        var url = "https://api.open-meteo.com/v1/elevation?latitude="
                + Number(latitudeText) + "&longitude=" + Number(longitudeText)
        var request = new XMLHttpRequest()
        request.onreadystatechange = function() {
            if (request.readyState !== XMLHttpRequest.DONE)
                return
            if (request.status >= 200 && request.status < 300) {
                try {
                    var parsed = JSON.parse(request.responseText)
                    if (!parsed.elevation || parsed.elevation.length < 1
                            || !isFinite(Number(parsed.elevation[0])))
                        throw new Error("Response has no valid elevation value")
                    root.demResult = parsed
                    root.demStatus = "CONNECTED / DATA RECEIVED"
                    root.demError = ""
                } catch (error) {
                    root.demStatus = "INVALID RESPONSE"
                    root.demError = String(error)
                }
            } else {
                root.demStatus = "CONNECTION FAILED"
                root.demError = "HTTP " + request.status + ": " + request.statusText
            }
        }
        request.open("GET", url)
        request.send()
    }

    function requestNotamApiTest() {
        var base = notamBaseUrl.trim()
        while (base.endsWith("/"))
            base = base.slice(0, -1)
        if (base.length === 0) {
            notamApiStatus = "CONFIGURATION ERROR"
            notamApiError = "Enter the provider API base URL."
            return
        }
        var apiKey = providerSecret("notam", notamApiKey)
        if (apiKey.length === 0) {
            notamApiStatus = "KEY REQUIRED"
            notamApiError = "Enter a token or save one to the system credential vault."
            return
        }
        notamApiStatus = "CONNECTING"
        notamApiError = ""
        notamApiCount = -1
        notamApiSample = ""
        var request = new XMLHttpRequest()
        request.timeout = 15000
        request.onreadystatechange = function() {
            if (request.readyState !== XMLHttpRequest.DONE)
                return
            if (request.status >= 200 && request.status < 300) {
                try {
                    var parsed = JSON.parse(request.responseText)
                    if (!parsed || !Array.isArray(parsed.results) || parsed.count === undefined)
                        throw new Error("Response does not match the expected NOTAC NOTAM search envelope.")
                    root.notamApiCount = Number(parsed.count)
                    if (parsed.results.length > 0) {
                        var first = parsed.results[0]
                        root.notamApiSample = String(first.number || first.id || "NOTICE")
                                + " | " + String(first.status || "STATUS UNKNOWN")
                                + " | " + String(first.text || "No source text returned").slice(0, 240)
                    } else {
                        root.notamApiSample = "Valid API response; no matching notices returned for the test query."
                    }
                    root.notamApiStatus = "CONNECTED / JSON VALID"
                    root.notamApiError = ""
                } catch (error) {
                    root.notamApiStatus = "INVALID RESPONSE"
                    root.notamApiError = String(error)
                }
            } else {
                root.notamApiStatus = request.status === 401 ? "AUTHENTICATION FAILED"
                    : request.status === 402 ? "API QUOTA EXHAUSTED"
                    : request.status === 403 ? "ACCESS DENIED"
                    : request.status === 429 ? "RATE LIMITED"
                    : "CONNECTION FAILED"
                root.notamApiError = "HTTP " + request.status + ": " + request.statusText
            }
        }
        request.ontimeout = function() {
            root.notamApiStatus = "TIMEOUT"
            root.notamApiError = "No response within 15 seconds."
        }
        request.onerror = function() {
            root.notamApiStatus = "NETWORK ERROR"
            root.notamApiError = "Request failed. Check network access, TLS and provider availability."
        }
        request.open("GET", base + "/notam/?country_code=US&status=active&page=1")
        request.setRequestHeader("Authorization", "Bearer " + apiKey)
        request.setRequestHeader("Accept", "application/json")
        request.send()
    }

    function statusColor(status) {
        if (status.indexOf("CONNECTED") === 0)
            return green
        if (status.indexOf("FAILED") >= 0 || status.indexOf("INVALID") >= 0)
            return red
        if (status.indexOf("NOT TESTED") >= 0 || status.indexOf("ACCESS") >= 0)
            return amber
        return cyan
    }

    Component.onCompleted: root.refreshCredentialStates()

    Rectangle { anchors.fill: parent; color: "transparent" }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: column.implicitHeight + 48
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: column
            x: 24
            y: 24
            width: parent.width - 48
            spacing: 14

            Column {
                width: parent.width
                spacing: 5
                Text { text: "EXTERNAL INTEGRATIONS"; color: root.text; font.family: "B612"; font.pixelSize: 22; font.bold: true }
                Text { text: "ADMINISTRATOR / LIVE SOURCE CONNECTIONS"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 10 }
            }

            Rectangle {
                width: parent.width
                height: 112
                radius: 4
                color: root.panel
                border.color: root.line
                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8
                    Text { text: "TEST LOCATION / WGS84"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 10; font.bold: true }
                    Row {
                        width: parent.width
                        spacing: 12
                        Column {
                            width: (parent.width - 12) / 2
                            spacing: 4
                            Text { text: "LATITUDE"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            TextField {
                                width: parent.width
                                height: 34
                                text: root.latitudeText
                                placeholderText: "e.g. 55.7558"
                                color: root.text
                                onTextChanged: root.latitudeText = text
                                background: Rectangle { color: "#07111B"; border.color: root.line; radius: 3 }
                            }
                        }
                        Column {
                            width: (parent.width - 12) / 2
                            spacing: 4
                            Text { text: "LONGITUDE"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            TextField {
                                width: parent.width
                                height: 34
                                text: root.longitudeText
                                placeholderText: "e.g. 37.6173"
                                color: root.text
                                onTextChanged: root.longitudeText = text
                                background: Rectangle { color: "#07111B"; border.color: root.line; radius: 3 }
                            }
                        }
                    }
                    Text { text: "For this initial test, enter a location manually. Route-based sampling is not connected yet."; color: root.muted; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                }
            }

            Grid {
                width: parent.width
                columns: 2
                spacing: 12

                Rectangle {
                    width: (column.width - 12) / 2
                    height: wxCard.implicitHeight + 24
                    radius: 4
                    color: root.panel
                    border.color: root.line
                    Column {
                        id: wxCard
                        x: 12; y: 12
                        width: parent.width - 24
                        spacing: 8
                        Text { text: "WX / OPEN-METEO"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                        Text { text: root.wxStatus; color: root.statusColor(root.wxStatus); font.family: "B612 Mono"; font.pixelSize: 9; wrapMode: Text.Wrap }
                        Text {
                            width: parent.width
                            text: root.wxResult
                                  ? "TIME " + root.wxResult.current.time + " UTC"
                                    + "\\nTEMP " + root.wxResult.current.temperature_2m + " °C"
                                    + "\\nWIND " + root.wxResult.current.wind_speed_10m + " km/h / DIR " + root.wxResult.current.wind_direction_10m + "°"
                                    + "\\nGUST " + root.wxResult.current.wind_gusts_10m + " km/h"
                                    + "\\nCLOUD " + root.wxResult.current.cloud_cover + "% / RH " + root.wxResult.current.relative_humidity_2m + "%"
                                    + "\\nPRESSURE " + root.wxResult.current.pressure_msl + " hPa"
                                  : "Live current model data and timestamp will appear here after a successful request."
                            color: root.text; font.family: "B612 Mono"; font.pixelSize: 9; wrapMode: Text.Wrap
                        }
                        Text { visible: root.wxError !== ""; width: parent.width; text: root.wxError; color: root.red; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                        Rectangle {
                            width: parent.width; height: 34; radius: 3; color: "#10283A"; border.color: root.cyan
                            Text { anchors.centerIn: parent; text: "FETCH LIVE WEATHER"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                            MouseArea { anchors.fill: parent; onClicked: root.requestWeather() }
                        }
                        Text { width: parent.width; text: "Provider: Open-Meteo public API. Prototype use only; no commercial licence or SLA is implied."; color: root.muted; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                    }
                }

                Rectangle {
                    width: (column.width - 12) / 2
                    height: demCard.implicitHeight + 24
                    radius: 4
                    color: root.panel
                    border.color: root.line
                    Column {
                        id: demCard
                        x: 12; y: 12
                        width: parent.width - 24
                        spacing: 8
                        Text { text: "TERRAIN / OPEN-METEO ELEVATION"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true; wrapMode: Text.Wrap }
                        Text { text: root.demStatus; color: root.statusColor(root.demStatus); font.family: "B612 Mono"; font.pixelSize: 9; wrapMode: Text.Wrap }
                        Text {
                            width: parent.width
                            text: root.demResult
                                  ? "COORDINATE " + root.latitudeText + ", " + root.longitudeText
                                    + "\\nELEVATION " + root.demResult.elevation[0] + " m"
                                  : "Point elevation will appear here after a successful request."
                            color: root.text; font.family: "B612 Mono"; font.pixelSize: 9; wrapMode: Text.Wrap
                        }
                        Text { visible: root.demError !== ""; width: parent.width; text: root.demError; color: root.red; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                        Rectangle {
                            width: parent.width; height: 34; radius: 3; color: "#10283A"; border.color: root.cyan
                            Text { anchors.centerIn: parent; text: "FETCH POINT ELEVATION"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                            MouseArea { anchors.fill: parent; onClicked: root.requestElevation() }
                        }
                        Text { width: parent.width; text: "This is a point query, not a terrain profile or obstacle database. Source resolution and vertical datum must be verified before operational use."; color: root.muted; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: notamColumn.implicitHeight + 24
                radius: 4
                color: root.panel
                border.color: root.line
                Column {
                    id: notamColumn
                    x: 12; y: 12
                    width: parent.width - 24
                    spacing: 8
                    Text { text: "NOTAM / AIP — RUSSIAN FEDERATION"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                    Text { text: root.notamStatus; color: root.amber; font.family: "B612 Mono"; font.pixelSize: 9 }
                    Text {
                        width: parent.width
                        text: "Official source: Centre for Aeronautical Information (CAI), State ATM Corporation. The official NOTAM service is identified, but a public machine-readable API, automated access terms and credentials have not been verified. No NOTAM feed is being parsed or treated as connected."
                        color: root.text; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap
                    }
                    Rectangle {
                        width: 240; height: 34; radius: 3; color: "#10283A"; border.color: root.cyan
                        Text { anchors.centerIn: parent; text: "OPEN OFFICIAL NOTAM PORTAL"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                        MouseArea { anchors.fill: parent; onClicked: Qt.openUrlExternally("https://aip.caica.ru/notam/") }
                    }
                    Text { width: parent.width; text: "Next integration step: confirm the official machine-readable delivery channel and access rights. Until then, NOTAM status remains UNAVAILABLE and must block operational release."; color: root.amber; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
                }
            }

            Rectangle {
                width: parent.width
                height: providerNodes.implicitHeight + 28
                radius: 4
                color: root.panel
                border.color: root.line
                Column {
                    id: providerNodes
                    x: 14; y: 14
                    width: parent.width - 28
                    spacing: 12
                    Text { text: "PROVIDER CONNECTION NODES"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 12; font.bold: true }
                    Text {
                        width: parent.width
                        text: "Setup: obtain the provider key, paste it into the masked field, select SAVE KEY, then test the connection. Keys are stored in the current Windows user Credential Manager, not in QSettings or the repository. Unsupported platforms fail closed."
                        color: root.text; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap
                    }

                    Rectangle {
                        width: parent.width
                        height: notacNode.implicitHeight + 24
                        color: "#101F2D"; border.color: root.line; radius: 4
                        Column {
                            id: notacNode
                            x: 12; y: 12; width: parent.width - 24; spacing: 7
                            Text { text: "NOTAM NODE / NOTAC REST JSON"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                            Text { text: "TEST PROVIDER · NOTAM dataset is FAA-sourced; this tests transport/auth/schema only, not Russian airspace coverage."; width: parent.width; color: root.amber; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
                            Text { text: "API BASE URL"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            TextField {
                                width: parent.width; height: 34
                                text: root.notamBaseUrl
                                onTextChanged: root.notamBaseUrl = text
                                color: root.text; selectByMouse: true
                                background: Rectangle { color: "#07111B"; border.color: root.line; radius: 3 }
                            }
                            Text { text: "API TOKEN · paste the secret key here"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            TextField {
                                width: parent.width; height: 34
                                text: root.notamApiKey
                                onTextChanged: root.notamApiKey = text
                                echoMode: TextInput.Password
                                passwordCharacter: "•"
                                placeholderText: "NOTAC token; save before restart"
                                color: root.text; selectByMouse: true
                                background: Rectangle { color: "#07111B"; border.color: root.line; radius: 3 }
                            }
                            Text { text: root.notamSecretSaved ? "KEY SAVED IN SYSTEM VAULT" : "NO SAVED KEY"; color: root.notamSecretSaved ? root.green : root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            Row { width: parent.width; spacing: 6
                                Rectangle { width: (parent.width - 12) / 3; height: 32; radius: 3; color: "#10283A"; border.color: root.cyan
                                    Text { anchors.centerIn: parent; text: "SAVE KEY"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.saveProviderSecret("notam") } }
                                Rectangle { width: (parent.width - 12) / 3; height: 32; radius: 3; color: "#241D20"; border.color: root.red
                                    Text { anchors.centerIn: parent; text: "DELETE KEY"; color: root.red; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.deleteProviderSecret("notam") } }
                                Rectangle { width: (parent.width - 12) / 3; height: 32; radius: 3; color: "#10283A"; border.color: root.cyan
                                    Text { anchors.centerIn: parent; text: "TEST API"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.requestNotamApiTest() } }
                            }
                            Text { text: root.notamApiStatus; color: root.statusColor(root.notamApiStatus); font.family: "B612 Mono"; font.pixelSize: 9; wrapMode: Text.Wrap }
                            Text { visible: root.notamApiError !== ""; text: root.notamApiError; width: parent.width; color: root.red; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                            Text { visible: root.notamApiCount >= 0; text: "MATCHING RECORDS: " + root.notamApiCount + "\\nSAMPLE: " + root.notamApiSample; width: parent.width; color: root.text; font.family: "B612 Mono"; font.pixelSize: 9; wrapMode: Text.Wrap }
                            Text { text: "API docs: https://notac.aero/api/ · token is sent in the Authorization: Bearer header, never in the URL."; width: parent.width; color: root.muted; font.family: "B612"; font.pixelSize: 9; wrapMode: Text.Wrap }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 104
                        color: "#101F2D"; border.color: root.line; radius: 4
                        Column {
                            x: 12; y: 12; width: parent.width - 24; spacing: 6
                            Text { text: "ICAO NOTAM API / FUTURE ALTERNATE"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                            Text { text: "Key field is provided for configuration planning. Endpoint parameters and Russian/UAS coverage must be verified before enabling a live test."; width: parent.width; color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
                            TextField {
                                width: parent.width; height: 32
                                text: root.icaoApiKey
                                onTextChanged: root.icaoApiKey = text
                                echoMode: TextInput.Password
                                passwordCharacter: "•"
                                placeholderText: "ICAO API key"
                                color: root.text
                                background: Rectangle { color: "#07111B"; border.color: root.line; radius: 3 }
                            }
                            Text { text: root.icaoSecretSaved ? "KEY SAVED IN SYSTEM VAULT" : "NO SAVED KEY"; color: root.icaoSecretSaved ? root.green : root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            Row { width: parent.width; spacing: 8
                                Rectangle { width: (parent.width - 8) / 2; height: 30; radius: 3; color: "#10283A"; border.color: root.cyan
                                    Text { anchors.centerIn: parent; text: "SAVE KEY"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.saveProviderSecret("icao") } }
                                Rectangle { width: (parent.width - 8) / 2; height: 30; radius: 3; color: "#241D20"; border.color: root.red
                                    Text { anchors.centerIn: parent; text: "DELETE KEY"; color: root.red; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.deleteProviderSecret("icao") } }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: mapNode.implicitHeight + 24
                        color: "#101F2D"; border.color: root.line; radius: 4
                        Column {
                            id: mapNode
                            x: 12; y: 12; width: parent.width - 24; spacing: 7
                            Text { text: "MAP NODE / MAPTILER"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                            Text { text: "Paste the MapTiler key issued for the BlueSky PRO application. This field does not replace or alter the currently configured map provider yet."; width: parent.width; color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
                            TextField {
                                width: parent.width; height: 34
                                text: root.mapTilerApiKey
                                onTextChanged: root.mapTilerApiKey = text
                                echoMode: TextInput.Password
                                passwordCharacter: "•"
                                placeholderText: "MapTiler API key"
                                color: root.text
                                background: Rectangle { color: "#07111B"; border.color: root.line; radius: 3 }
                            }
                            Text { text: root.mapTilerSecretSaved ? "KEY SAVED IN SYSTEM VAULT" : "NO SAVED KEY"; color: root.mapTilerSecretSaved ? root.green : root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            Row { width: parent.width; spacing: 8
                                Rectangle { width: (parent.width - 8) / 2; height: 30; radius: 3; color: "#10283A"; border.color: root.cyan
                                    Text { anchors.centerIn: parent; text: "SAVE KEY"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.saveProviderSecret("maptiler") } }
                                Rectangle { width: (parent.width - 8) / 2; height: 30; radius: 3; color: "#241D20"; border.color: root.red
                                    Text { anchors.centerIn: parent; text: "DELETE KEY"; color: root.red; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.deleteProviderSecret("maptiler") } }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: wxNode.implicitHeight + 24
                        color: "#101F2D"; border.color: root.line; radius: 4
                        Column {
                            id: wxNode
                            x: 12; y: 12; width: parent.width - 24; spacing: 7
                            Text { text: "WEATHER NODE / OPEN-METEO"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                            Text { text: "The current prototype uses public Open-Meteo endpoints without an API key. Do not paste a paid-provider key here expecting it to affect the current request; commercial-provider adapter selection is not implemented."; width: parent.width; color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
                            TextField {
                                width: parent.width; height: 34
                                text: root.wxCommercialApiKey
                                onTextChanged: root.wxCommercialApiKey = text
                                echoMode: TextInput.Password
                                passwordCharacter: "•"
                                placeholderText: "Future commercial WX key"
                                color: root.text
                                background: Rectangle { color: "#07111B"; border.color: root.line; radius: 3 }
                            }
                            Text { text: root.wxSecretSaved ? "KEY SAVED IN SYSTEM VAULT" : "NO SAVED KEY"; color: root.wxSecretSaved ? root.green : root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                            Row { width: parent.width; spacing: 8
                                Rectangle { width: (parent.width - 8) / 2; height: 30; radius: 3; color: "#10283A"; border.color: root.cyan
                                    Text { anchors.centerIn: parent; text: "SAVE KEY"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.saveProviderSecret("weather") } }
                                Rectangle { width: (parent.width - 8) / 2; height: 30; radius: 3; color: "#241D20"; border.color: root.red
                                    Text { anchors.centerIn: parent; text: "DELETE KEY"; color: root.red; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                                    MouseArea { anchors.fill: parent; onClicked: root.deleteProviderSecret("weather") } }
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        text: "SECURITY: secrets are stored in Windows Credential Manager. They are not written to QSettings or the repository. ICAO and commercial-weather keys are stored for future adapter work but are not used by a live adapter yet. MapTiler is loaded at startup from the vault when no environment-variable key is supplied."
                        color: root.amber; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap
                    }
                }
            }

            Text {
                width: parent.width
                text: "CREDENTIAL VAULT: " + root.credentialStoreStatus + (root.credentialStoreMessage === "" ? "" : "\n" + root.credentialStoreMessage) + "\nDATA SAFETY: a successful HTTP response proves transport only. Source timestamp, coverage, integrity, freshness, licence, vertical datum and schema must be validated separately. These initial requests do not authorise flight release."
                color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap
            }
        }
    }
}
