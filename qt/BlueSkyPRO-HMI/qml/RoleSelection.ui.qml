import QtQuick

Item {
    id: root
    width: 1920
    height: 1080

    // BlueSky PRO — role selection screen, visual HMI layer only.
    property string selectedRole: "ADMIN"
    property string displayDate: "14 Oct 2024"
    property string displayTime: "12:45 (UTC+3)"
    property string languageCode: "RU"

    signal continueRequested(string role)
    signal languageRequested(string languageCode)

    readonly property color backgroundColor: "#1D2A38"
    readonly property color panelColor: "#202D3B"
    readonly property color selectedColor: "#0B3048"
    readonly property color cyan: "#00B9F0"
    readonly property color textColor: "#E8EEF5"
    readonly property color secondaryText: "#B7C4D2"
    readonly property color borderColor: "#46596B"

    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#223140" }
            GradientStop { position: 1.0; color: "#1A2633" }
        }
    }

    // Header
    Row {
        id: brand
        x: 34
        y: 28
        spacing: 16

        Canvas {
            width: 52
            height: 48
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.fillStyle = root.cyan
                ctx.beginPath()
                ctx.moveTo(2, 4)
                ctx.lineTo(24, 4)
                ctx.lineTo(47, 4)
                ctx.lineTo(47, 12)
                ctx.lineTo(30, 25)
                ctx.lineTo(2, 12)
                ctx.closePath()
                ctx.fill()
                ctx.beginPath()
                ctx.moveTo(7, 18)
                ctx.lineTo(26, 28)
                ctx.lineTo(45, 18)
                ctx.lineTo(29, 42)
                ctx.lineTo(23, 42)
                ctx.closePath()
                ctx.fill()
            }
        }

        Column {
            spacing: 0
            Text {
                text: "BlueSky PRO"
                color: root.textColor
                font.family: "B612"
                font.pixelSize: 30
                font.bold: true
            }
            Text {
                text: "F L I G H T   P L A N N I N G"
                color: root.secondaryText
                font.family: "B612"
                font.pixelSize: 10
            }
        }
    }

    Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 32
        anchors.rightMargin: 32
        spacing: 22

        Text {
            text: root.displayDate
            color: root.textColor
            font.family: "B612"
            font.pixelSize: 14
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.displayTime
            color: root.textColor
            font.family: "B612"
            font.pixelSize: 14
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            width: 1
            height: 34
            color: root.borderColor
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            width: 38
            height: 38
            radius: 19
            color: "transparent"
            border.color: root.borderColor
            anchors.verticalCenter: parent.verticalCenter

            Text {
                anchors.centerIn: parent
                text: "◎"
                color: root.textColor
                font.pixelSize: 22
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.languageCode = root.languageCode === "RU" ? "EN" : "RU"
                    root.languageRequested(root.languageCode)
                }
            }
        }
    }

    Column {
        id: titleBlock
        anchors.horizontalCenter: parent.horizontalCenter
        y: 150
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Авторизация"
            color: root.textColor
            font.family: "B612"
            font.pixelSize: 30
            font.bold: true
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Выберите роль"
            color: root.secondaryText
            font.family: "B612"
            font.pixelSize: 20
        }
    }

    Row {
        id: roleCards
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 34
        anchors.rightMargin: 34
        y: 260
        height: Math.min(500, parent.height * 0.50)
        spacing: 14

        Repeater {
            model: [
                {
                    role: "ADMIN",
                    title: "АДМИНИСТРАТОР",
                    icon: "●",
                    description: ["Управление системой", "Пользователи", "Настройки", "Интеграции"]
                },
                {
                    role: "ENGINEER",
                    title: "ИНЖЕНЕР",
                    icon: "⚙",
                    description: ["Парк БПЛА", "Оборудование", "Техническое состояние", "Документация"]
                },
                {
                    role: "TECHNICIAN",
                    title: "ТЕХНИК",
                    icon: "🔧",
                    description: ["Выполнение работ", "Чек-листы", "Отчеты", "Статус оборудования"]
                },
                {
                    role: "PILOT",
                    title: "ПИЛОТ",
                    icon: "✈",
                    description: ["Управление полётом", "Планирование миссий", "Телеметрия", "Выполнение заданий"]
                }
            ]

            delegate: Item {
                width: (roleCards.width - roleCards.spacing * 3) / 4
                height: roleCards.height

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: root.selectedRole === modelData.role ? root.selectedColor : root.panelColor
                    border.color: root.selectedRole === modelData.role ? root.cyan : root.borderColor
                    border.width: root.selectedRole === modelData.role ? 2 : 1

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 5
                        radius: 5
                        color: "transparent"
                        border.color: root.selectedRole === modelData.role ? "#334DB9F0" : "transparent"
                        border.width: 1
                    }

                    Column {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 14

                        Item {
                            width: parent.width
                            height: 105

                            Text {
                                anchors.centerIn: parent
                                text: modelData.icon
                                color: root.selectedRole === modelData.role ? root.cyan : "#AAB8C8"
                                font.family: "Arial"
                                font.pixelSize: modelData.role === "ADMIN" ? 58 : 54
                            }
                        }

                        Text {
                            width: parent.width
                            text: modelData.title
                            color: root.selectedRole === modelData.role ? root.cyan : root.textColor
                            font.family: "B612"
                            font.pixelSize: 19
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                            minimumPixelSize: 13
                            fontSizeMode: Text.Fit
                        }

                        Column {
                            width: parent.width
                            spacing: 5

                            Repeater {
                                model: modelData.description

                                Text {
                                    width: parent.width
                                    text: modelData
                                    color: root.secondaryText
                                    font.family: "B612"
                                    font.pixelSize: 15
                                    horizontalAlignment: Text.AlignHCenter
                                    wrapMode: Text.Wrap
                                }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.selectedRole = modelData.role
                    }
                }
            }
        }
    }

    Rectangle {
        id: continueButton
        width: 320
        height: 58
        radius: 6
        anchors.horizontalCenter: parent.horizontalCenter
        y: roleCards.y + roleCards.height + 20
        color: root.selectedColor
        border.color: root.cyan
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 22

            Text {
                text: "Продолжить"
                color: root.textColor
                font.family: "B612"
                font.pixelSize: 18
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "→"
                color: root.textColor
                font.pixelSize: 30
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.continueRequested(root.selectedRole)
        }
    }
}
