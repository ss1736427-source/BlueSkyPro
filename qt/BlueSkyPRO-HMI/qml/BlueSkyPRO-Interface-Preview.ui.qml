import QtQuick

Item {
    id: root
    width: 1920
    height: 1080

    // BlueSky PRO — standalone full-interface visual preview.
    // This file is intentionally self-contained for Qt Design Studio preview.
    property color bg: "#050A12"
    property color panel: "#08111D"
    property color card: "#0C1725"
    property color active: "#111F30"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color muted: "#7F7F7F"
    property color cyan: "#32FFFF"
    property color green: "#64FF00"
    property color amber: "#FFD339"
    property color red: "#FF1E14"
    property bool rightPanelOpen: true

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    // HEADER
    Rectangle {
        x: 0; y: 0; width: parent.width; height: 86
        color: root.panel
    }

    Rectangle { x: 0; y: 0; width: parent.width; height: 1; color: root.cyan }
    Rectangle { x: 0; y: 85; width: parent.width; height: 1; color: root.cyan }
    Rectangle { x: 0; y: 0; width: 1; height: 86; color: root.cyan }
    Rectangle { x: parent.width - 1; y: 0; width: 1; height: 86; color: root.cyan }

    Text {
        x: 20; y: 24
        text: "BlueSky PRO"
        color: root.text
        font.family: "B612"
        font.pixelSize: 22
        font.bold: true
    }

    Text { x: 190; y: 18; text: "ETD"; color: root.muted; font.family: "B612"; font.pixelSize: 10 }
    Text { x: 190; y: 38; text: "10:30"; color: root.secondary; font.family: "B612"; font.pixelSize: 18; font.bold: true }
    Text { x: 330; y: 18; text: "TOT"; color: root.muted; font.family: "B612"; font.pixelSize: 10 }
    Text { x: 330; y: 38; text: "—"; color: root.secondary; font.family: "B612"; font.pixelSize: 18; font.bold: true }
    Text { x: 470; y: 18; text: "TRIP"; color: root.muted; font.family: "B612"; font.pixelSize: 10 }
    Text { x: 470; y: 38; text: "—"; color: root.secondary; font.family: "B612"; font.pixelSize: 18; font.bold: true }
    Text { x: 610; y: 18; text: "ETA"; color: root.muted; font.family: "B612"; font.pixelSize: 10 }
    Text { x: 610; y: 38; text: "11:48"; color: root.secondary; font.family: "B612"; font.pixelSize: 18; font.bold: true }
    Text { x: 750; y: 18; text: "READY"; color: root.muted; font.family: "B612"; font.pixelSize: 10 }
    Text { x: 750; y: 38; text: "READY"; color: root.green; font.family: "B612"; font.pixelSize: 18; font.bold: true }
    Text { x: 890; y: 18; text: "WARNING"; color: root.muted; font.family: "B612"; font.pixelSize: 10 }
    Text { x: 890; y: 38; text: "!"; color: root.amber; font.family: "B612"; font.pixelSize: 18; font.bold: true }

    Rectangle {
        x: parent.width - 178; y: 14; width: 150; height: 58
        color: "transparent"; border.color: "#111F30"; border.width: 1
    }
    Text { x: parent.width - 150; y: 28; text: "●"; color: root.secondary; font.pixelSize: 22 }
    Text { x: parent.width - 112; y: 31; text: "OPERATOR"; color: root.secondary; font.family: "B612"; font.pixelSize: 11; font.bold: true }

    // WORKSPACE
    Rectangle {
        x: 0; y: 86; width: 300; height: parent.height - 86 - 136
        color: root.panel
    }
    Rectangle {
        x: 300; y: 86; width: parent.width - 640; height: parent.height - 86 - 136
        color: "#07101B"
    }
    Rectangle {
        x: parent.width - 340; y: 86; width: 340; height: parent.height - 86 - 136
        color: root.panel
        visible: root.rightPanelOpen
    }

    Rectangle { x: 299; y: 86; width: 1; height: parent.height - 86 - 136; color: root.muted }
    Rectangle { x: parent.width - 340; y: 86; width: 1; height: parent.height - 86 - 136; color: root.muted; visible: root.rightPanelOpen }

    Text { x: 18; y: 106; text: "Миссиия"; color: root.secondary; font.family: "B612"; font.pixelSize: 13; font.bold: true }
    Text { x: 18; y: 142; text: "BS-260920-A-001"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 16; font.bold: true }
    Text { x: 18; y: 168; text: "Area Survey"; color: root.text; font.family: "B612"; font.pixelSize: 13 }
    Text { x: 18; y: 194; text: "4 UAV · BVLOS · wind-aware"; color: root.secondary; font.family: "B612"; font.pixelSize: 11 }
    Text { x: 18; y: 238; text: "MISSION TEMPLATES"; color: root.muted; font.family: "B612"; font.pixelSize: 10 }
    Text { x: 18; y: 266; text: "ROUTE"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 18; y: 292; text: "ALTITUDE"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 18; y: 318; text: "SPEED"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 18; y: 344; text: "OFFSET"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 18; y: 388; text: "WEATHER"; color: root.secondary; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 18; y: 414; text: "OBSTACLES"; color: root.secondary; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 18; y: 440; text: "AIRSPACE"; color: root.secondary; font.family: "B612 Mono"; font.pixelSize: 11 }

    Text { x: 0; width: parent.width; anchors.verticalCenter: undefined; y: 120; text: "FLIGHT CHART"; color: root.secondary; font.family: "B612"; font.pixelSize: 16; font.bold: true; horizontalAlignment: Text.AlignHCenter }
    Text { x: 300; y: 155; width: parent.width - 640; text: "MAP DESIGN / ROUTE / WEATHER / AIRSPACE"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter }

    Rectangle {
        x: 390; y: 245; width: parent.width - 780; height: 300
        color: "#08111D"; border.color: "#202020"; border.width: 1
    }
    Text { x: 390; y: 360; width: parent.width - 780; text: "FLIGHT CHART"; color: root.secondary; font.family: "B612"; font.pixelSize: 18; font.bold: true; horizontalAlignment: Text.AlignHCenter }
    Text { x: 390; y: 394; width: parent.width - 780; text: "PRIMARY MAP WORKSPACE"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 10; horizontalAlignment: Text.AlignHCenter }

    Text { x: parent.width - 322; y: 106; text: "MISSION ACTIONS"; color: root.secondary; font.family: "B612"; font.pixelSize: 13; font.bold: true; visible: root.rightPanelOpen }
    Text { x: parent.width - 322; y: 142; text: "CHECKLIST 5/8"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 12; visible: root.rightPanelOpen }
    Text { x: parent.width - 322; y: 174; text: "⚠ CORRECTIONS REQUIRED"; color: root.amber; font.family: "B612 Mono"; font.pixelSize: 10; visible: root.rightPanelOpen }
    Text { x: parent.width - 322; y: 206; text: "MISSION NOT READY"; color: root.amber; font.family: "B612"; font.pixelSize: 13; font.bold: true; visible: root.rightPanelOpen }
    Rectangle { x: parent.width - 322; y: 246; width: 250; height: 42; color: root.card; border.color: root.muted; border.width: 1; visible: root.rightPanelOpen }
    Text { x: parent.width - 322; y: 259; width: 250; text: "SEND FLIGHT PLAN"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter; visible: root.rightPanelOpen }
    Rectangle { x: parent.width - 322; y: 304; width: 250; height: 42; color: root.card; border.color: root.red; border.width: 1; visible: root.rightPanelOpen }
    Text { x: parent.width - 322; y: 317; width: 250; text: "START MISSION"; color: root.red; font.family: "B612 Mono"; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter; visible: root.rightPanelOpen }

    // UAV STATUS
    Rectangle { x: 0; y: parent.height - 136; width: parent.width; height: 82; color: root.panel; border.color: root.muted; border.width: 1 }
    Text { x: 18; y: parent.height - 118; text: "UAV STATUS"; color: root.secondary; font.family: "B612"; font.pixelSize: 11; font.bold: true }
    Text { x: 18; y: parent.height - 92; text: "UAV-01  READY 96%"; color: root.green; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 270; y: parent.height - 92; text: "UAV-02  READY 94%"; color: root.green; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 522; y: parent.height - 92; text: "UAV-03  CHECK 91%"; color: root.amber; font.family: "B612 Mono"; font.pixelSize: 11 }
    Text { x: 774; y: parent.height - 92; text: "UAV-04  READY 95%"; color: root.green; font.family: "B612 Mono"; font.pixelSize: 11 }

    // BOTTOM TOOLBAR — current locked intermediate variant
    Rectangle { x: 0; y: parent.height - 54; width: parent.width; height: 54; color: root.bg }
    Rectangle { x: 0; y: parent.height - 54; width: parent.width; height: 1; color: root.cyan }
    Rectangle { x: 0; y: parent.height - 1; width: parent.width; height: 1; color: root.cyan }
    Rectangle { x: 0; y: parent.height - 54; width: 1; height: 54; color: root.cyan }
    Rectangle { x: parent.width - 1; y: parent.height - 54; width: 1; height: 54; color: root.cyan }

    Rectangle { x: 20; y: parent.height - 45; width: 108; height: 38; color: root.card; border.color: root.muted; border.width: 1 }
    Text { x: 20; y: parent.height - 34; width: 108; text: "◀ LEFT"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }

    Rectangle { x: 138; y: parent.height - 45; width: 110; height: 38; color: root.card; border.color: root.muted; border.width: 1 }
    Text { x: 138; y: parent.height - 34; width: 110; text: "UAV"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }

    Rectangle { x: 258; y: parent.height - 45; width: 110; height: 38; color: root.active; border.color: root.cyan; border.width: 1 }
    Text { x: 258; y: parent.height - 34; width: 110; text: "MAP"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }

    Rectangle { x: 378; y: parent.height - 45; width: 110; height: 38; color: root.card; border.color: root.muted; border.width: 1 }
    Text { x: 378; y: parent.height - 34; width: 110; text: "ADMIN"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }

    Rectangle { x: 498; y: parent.height - 45; width: 110; height: 38; color: root.card; border.color: root.muted; border.width: 1 }
    Text { x: 498; y: parent.height - 34; width: 110; text: "FPV"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }

    Rectangle { x: 618; y: parent.height - 45; width: 150; height: 38; color: root.card; border.color: root.muted; border.width: 1 }
    Text { x: 618; y: parent.height - 34; width: 150; text: "VIRTUAL FLT"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }

    Rectangle { x: 1370; y: parent.height - 45; width: 108; height: 38; color: root.card; border.color: root.muted; border.width: 1 }
    Text { x: 1370; y: parent.height - 34; width: 108; height: 20; text: root.rightPanelOpen ? "RIGHT ▶" : "RIGHT ◀"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }
    MouseArea { x: 1370; y: parent.height - 45; width: 108; height: 38; onClicked: root.rightPanelOpen = !root.rightPanelOpen }

    Rectangle { x: 1498; y: parent.height - 45; width: 1; height: 38; color: root.cyan }
    Text { x: 1520; y: parent.height - 34; width: 72; text: "HH:MM"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 13; font.bold: true; horizontalAlignment: Text.AlignHCenter }
    Text { x: 1620; y: parent.height - 34; width: 48; text: "☰"; color: root.text; font.family: "B612 Mono"; font.pixelSize: 15; font.bold: true; horizontalAlignment: Text.AlignHCenter }

    Text {
        x: 20; y: parent.height - 4
        text: "DS / FULL INTERFACE PREVIEW / 001"
        color: root.muted
        font.family: "IBM Plex Sans Condensed"
        font.pixelSize: 9
    }
}
