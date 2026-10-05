import QtQuick

Item {
    id: root
    width: 1920
    height: 1080

    property string taskText: ""
    property var selectedTemplateIndices: []
    property string keyboardLanguage: "RU"
    property bool keyboardVisible: true

    signal missionSetRequested(var templateIndices, string taskText, var templateIds)

    readonly property color bg: "#06111F"
    readonly property color panel: "#0A1B2D"
    readonly property color card: "#0B2035"
    readonly property color cyan: "#20C8F4"
    readonly property color green: "#55F28A"
    readonly property color textColor: "#E5F0FA"
    readonly property color muted: "#9FB4C9"
    readonly property color line: "#214A68"

    function toggleTemplate(index) {
        var next = selectedTemplateIndices.slice()
        var at = next.indexOf(index)
        if (at >= 0)
            next.splice(at, 1)
        else
            next.push(index)
        selectedTemplateIndices = next
    }

    function insertKey(value) {
        taskInput.forceActiveFocus()
        if (value === "BACKSPACE") {
            if (taskInput.cursorPosition > 0) {
                var p = taskInput.cursorPosition
                taskInput.remove(p - 1, p)
            }
        } else if (value === "SPACE") {
            taskInput.insert(taskInput.cursorPosition, " ")
        } else if (value === "ENTER") {
            taskInput.insert(taskInput.cursorPosition, "\n")
        } else {
            taskInput.insert(taskInput.cursorPosition, value)
        }
        taskText = taskInput.text
    }

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    // Header / brand
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: Math.max(66, parent.height * 0.085)
        color: "#071522"
        border.color: root.line
        border.width: 1

        Text {
            anchors.left: parent.left
            anchors.leftMargin: parent.width * 0.045
            anchors.verticalCenter: parent.verticalCenter
            text: "BlueSky PRO"
            color: root.textColor
            font.family: "B612"
            font.pixelSize: Math.max(22, Math.min(32, parent.height * 0.38))
            font.bold: true
        }

        Text {
            anchors.centerIn: parent
            text: "СОЗДАНИЕ ЗАДАЧИ"
            color: root.textColor
            font.family: "B612"
            font.pixelSize: Math.max(18, Math.min(28, parent.height * 0.32))
            font.bold: true
            font.letterSpacing: 1
        }

        Text {
            anchors.right: parent.right
            anchors.rightMargin: parent.width * 0.045
            anchors.verticalCenter: parent.verticalCenter
            text: "PILOT  /  13 ШАБЛОНОВ"
            color: root.cyan
            font.family: "B612"
            font.pixelSize: Math.max(12, Math.min(18, parent.height * 0.22))
        }
    }

    // Task prompt field grows with entered text, while remaining inside the upper workspace.
    Rectangle {
        id: promptFrame
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.105
        anchors.leftMargin: parent.width * 0.045
        anchors.rightMargin: parent.width * 0.045
        height: Math.min(parent.height * 0.18, Math.max(parent.height * 0.075, taskInput.contentHeight + 28))
        radius: 10
        color: root.panel
        border.color: taskInput.activeFocus ? root.cyan : root.line
        border.width: taskInput.activeFocus ? 2 : 1

        TextEdit {
            id: taskInput
            anchors.fill: parent
            anchors.margins: 16
            text: root.taskText
            color: root.textColor
            selectionColor: "#24678D"
            selectedTextColor: "#FFFFFF"
            font.family: "Noto Sans"
            font.pixelSize: Math.max(18, Math.min(30, root.width * 0.016))
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            activeFocusOnPress: true
            onTextChanged: root.taskText = text

            Text {
                anchors.fill: parent
                visible: taskInput.text.length === 0
                text: "Опишите задачу… например: обследовать территорию площадью 5 км²"
                color: root.muted
                font: taskInput.font
                wrapMode: Text.Wrap
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            width: 34
            height: 34
            radius: 17
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            color: "#173B57"
            visible: taskInput.text.length > 0

            Text {
                anchors.centerIn: parent
                text: "×"
                color: root.textColor
                font.pixelSize: 24
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    taskInput.text = ""
                    root.taskText = ""
                    taskInput.forceActiveFocus()
                }
            }
        }
    }

    Text {
        id: templateCaption
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0.045
        anchors.top: promptFrame.bottom
        anchors.topMargin: 12
        text: "ВЫБЕРИТЕ ШАБЛОНЫ МИССИЙ"
        color: root.muted
        font.family: "B612"
        font.pixelSize: Math.max(11, Math.min(15, root.width * 0.008))
        font.bold: true
        font.letterSpacing: 1
    }

    // All 13 mission templates are shown at entry; multiple templates may be selected.
    Grid {
        id: templates
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: parent.width * 0.045
        anchors.rightMargin: parent.width * 0.045
        anchors.top: templateCaption.bottom
        anchors.topMargin: 10
        property int columnCount: root.width >= 1500 ? 7 : root.width >= 1000 ? 5 : 3
        columns: columnCount
        spacing: 12
        property real tileWidth: (width - spacing * (columnCount - 1)) / columnCount
        property real tileHeight: Math.max(48, Math.min(76, root.height * 0.075))

        Repeater {
            model: [
                { id: "MAPPING", shortName: "КАРТОГРАФИЯ", fullName: "Картографирование территории" },
                { id: "3D_MAPPING", shortName: "3D-МОДЕЛЬ", fullName: "3D-картография и реконструкция" },
                { id: "INSPECTION", shortName: "ИНСПЕКЦИЯ", fullName: "Инспекция объектов и инфраструктуры" },
                { id: "CONSTRUCTION_MONITORING", shortName: "СТРОЙКА", fullName: "Мониторинг строительства" },
                { id: "PERIMETER_MONITORING", shortName: "ПЕРИМЕТР", fullName: "Мониторинг территории и периметра" },
                { id: "SEARCH_AND_RESCUE", shortName: "ПОИСК / SAR", fullName: "Поиск и спасение" },
                { id: "FIRE_EMERGENCY", shortName: "ПОЖАР / ЧС", fullName: "Пожарный мониторинг и ЧС" },
                { id: "ENVIRONMENTAL_MONITORING", shortName: "ЭКОЛОГИЯ", fullName: "Экологический и природный мониторинг" },
                { id: "AGRICULTURE", shortName: "АГРО", fullName: "Сельское хозяйство" },
                { id: "DELIVERY", shortName: "ДОСТАВКА", fullName: "Доставка грузов" },
                { id: "COMMUNICATION_RELAY", shortName: "РЕТРАНСЛЯЦИЯ", fullName: "Ретрансляция связи" },
                { id: "AERIAL_MEDIA", shortName: "АЭРОСЪЁМКА", fullName: "Аэрофотосъёмка и медиапроизводство" },
                { id: "C_UAS", shortName: "C-UAS", fullName: "Обнаружение и наблюдение за БПЛА" }
            ]

            delegate: Rectangle {
                required property int index
                required property var modelData
                width: templates.tileWidth
                height: templates.tileHeight
                radius: 8
                color: root.selectedTemplateIndices.indexOf(index) >= 0 ? "#103A35" : root.card
                border.color: root.selectedTemplateIndices.indexOf(index) >= 0 ? root.green : root.line
                border.width: root.selectedTemplateIndices.indexOf(index) >= 0 ? 2 : 1

                Column {
                    anchors.centerIn: parent
                    width: parent.width - 12
                    spacing: 4

                    Text {
                        width: parent.width
                        text: modelData.shortName
                        color: root.selectedTemplateIndices.indexOf(index) >= 0 ? root.green : root.textColor
                        font.family: "B612"
                        font.pixelSize: Math.max(10, Math.min(16, root.width * 0.0085))
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    Text {
                        width: parent.width
                        visible: templates.tileHeight > 60
                        text: modelData.fullName
                        color: root.muted
                        font.family: "Noto Sans"
                        font.pixelSize: Math.max(9, Math.min(11, root.width * 0.006))
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                }

                Text {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.rightMargin: 7
                    anchors.topMargin: 4
                    visible: root.selectedTemplateIndices.indexOf(index) >= 0
                    text: "✓"
                    color: root.green
                    font.pixelSize: 14
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.toggleTemplate(index)
                }
            }
        }
    }

    Rectangle {
        id: continueButton
        anchors.right: parent.right
        anchors.rightMargin: parent.width * 0.045
        anchors.top: templates.bottom
        anchors.topMargin: 12
        width: Math.max(210, Math.min(300, parent.width * 0.19))
        height: Math.max(42, Math.min(54, parent.height * 0.055))
        radius: 7
        color: root.selectedTemplateIndices.length > 0 ? "#103A35" : "#172536"
        border.color: root.selectedTemplateIndices.length > 0 ? root.green : root.line
        border.width: 1
        opacity: root.selectedTemplateIndices.length > 0 ? 1 : 0.55

        Row {
            anchors.centerIn: parent
            spacing: 16
            Text {
                text: "ПРОДОЛЖИТЬ"
                color: root.textColor
                font.family: "B612"
                font.pixelSize: 14
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: "→"
                color: root.green
                font.pixelSize: 22
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        MouseArea {
            anchors.fill: parent
            enabled: root.selectedTemplateIndices.length > 0
            onClicked: root.missionSetRequested(root.selectedTemplateIndices, root.taskText, root.selectedTemplateIndices.map(function(i) { return templates.children[0].model[i].id }))
        }
    }

    // On-screen Russian keyboard is visible by default for tablet operation.
    Rectangle {
        id: keyboardPanel
        visible: root.keyboardVisible
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Math.min(parent.height * 0.43, 470)
        color: "#0D2034"
        border.color: root.line
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10

            Repeater {
                model: root.keyboardLanguage === "RU" ? [
                    ["Й","Ц","У","К","Е","Н","Г","Ш","Щ","З","Х","BACKSPACE"],
                    ["Ф","Ы","В","А","П","Р","О","Л","Д","Ж","Э","ENTER"],
                    ["SHIFT","Я","Ч","С","М","И","Т","Ь","Б","Ю","Ё","SHIFT"]
                ] : [
                    ["Q","W","E","R","T","Y","U","I","O","P","BACKSPACE"],
                    ["A","S","D","F","G","H","J","K","L","ENTER"],
                    ["SHIFT","Z","X","C","V","B","N","M","SHIFT"]
                ]

                delegate: Row {
                    id: keyRow
                    required property var modelData
                    width: parent.width
                    height: (parent.height - 56) / 4
                    spacing: 8

                    Repeater {
                        model: modelData
                        delegate: Rectangle {
                            required property string modelData
                            width: (keyRow.width - keyRow.spacing * (keyRow.modelData.length - 1)) / keyRow.modelData.length
                            height: parent.height
                            radius: 7
                            color: modelData === "BACKSPACE" || modelData === "ENTER" || modelData === "SHIFT" ? "#142D45" : "#263D54"
                            border.color: "#304E68"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: modelData === "BACKSPACE" ? "⌫" : modelData === "ENTER" ? "↵" : modelData === "SHIFT" ? "⇧" : modelData
                                color: root.textColor
                                font.family: "Noto Sans"
                                font.pixelSize: Math.max(14, Math.min(24, root.width * 0.013))
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (modelData === "SHIFT") {
                                        root.keyboardLanguage = root.keyboardLanguage === "RU" ? "EN" : "RU"
                                    } else {
                                        root.insertKey(modelData)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Row {
                width: parent.width
                height: 44
                spacing: 8

                Rectangle {
                    width: parent.width * 0.12
                    height: parent.height
                    radius: 7
                    color: "#142D45"
                    border.color: "#304E68"
                    Text { anchors.centerIn: parent; text: "?123"; color: root.textColor; font.pixelSize: 15 }
                    MouseArea { anchors.fill: parent; onClicked: root.insertKey("-") }
                }
                Rectangle {
                    width: parent.width * 0.09
                    height: parent.height
                    radius: 7
                    color: "#142D45"
                    border.color: "#304E68"
                    Text { anchors.centerIn: parent; text: "RU"; color: root.textColor; font.pixelSize: 15 }
                    MouseArea { anchors.fill: parent; onClicked: root.keyboardLanguage = root.keyboardLanguage === "RU" ? "EN" : "RU" }
                }
                Rectangle {
                    width: parent.width * 0.55
                    height: parent.height
                    radius: 7
                    color: "#263D54"
                    border.color: "#304E68"
                    Text { anchors.centerIn: parent; text: "Пробел"; color: root.textColor; font.pixelSize: 16 }
                    MouseArea { anchors.fill: parent; onClicked: root.insertKey("SPACE") }
                }
                Rectangle {
                    width: parent.width * 0.09
                    height: parent.height
                    radius: 7
                    color: "#142D45"
                    border.color: "#304E68"
                    Text { anchors.centerIn: parent; text: "."; color: root.textColor; font.pixelSize: 18 }
                    MouseArea { anchors.fill: parent; onClicked: root.insertKey(".") }
                }
                Rectangle {
                    width: parent.width * 0.12
                    height: parent.height
                    radius: 7
                    color: "#142D45"
                    border.color: "#304E68"
                    Text { anchors.centerIn: parent; text: root.keyboardVisible ? "Скрыть" : "⌨"; color: root.textColor; font.pixelSize: 13 }
                    MouseArea { anchors.fill: parent; onClicked: root.keyboardVisible = !root.keyboardVisible }
                }
            }
        }
    }
    Rectangle {
        visible: !root.keyboardVisible
        z: 10
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 20
        anchors.bottomMargin: 18
        width: 58
        height: 44
        radius: 7
        color: "#142D45"
        border.color: root.cyan
        Text {
            anchors.centerIn: parent
            text: "⌨"
            color: root.textColor
            font.pixelSize: 22
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.keyboardVisible = true
                taskInput.forceActiveFocus()
            }
        }
    }

}
