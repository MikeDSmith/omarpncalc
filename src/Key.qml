import QtQuick
import QtQuick.Layouts
import QtQuick.Window

// One keypad button. Digits sit almost flush with the page, the operator
// column lifts a step lighter, and enter fills with the accent colour.
Rectangle {
    id: key

    property string label
    property var action
    // "digit" | "op" | "enter"
    property string kind: "digit"
    property color labelColor: kind === "enter" ? theme.background : theme.foreground

    // Follow the window's design-size scale (see Main.qml).
    readonly property real ui: Window.window ? Window.window.uiScale : 1
    function s(px) { return Math.max(1, Math.round(px * ui)) }
    function alpha(c, a) { return Qt.rgba(c.r, c.g, c.b, a) }

    Layout.fillWidth: true
    Layout.fillHeight: true
    radius: s(10)
    color: kind === "enter"
           ? (mouse.pressed ? Qt.darker(theme.accent, 1.15) : theme.accent)
           : alpha(theme.foreground, (kind === "op" ? 0.15 : 0.05)
                   + (mouse.pressed ? 0.10 : mouse.containsMouse ? 0.04 : 0))
    Behavior on color { ColorAnimation { duration: 80 } }

    Text {
        anchors.centerIn: parent
        text: key.label
        color: key.labelColor
        font.family: "monospace"
        font.pixelSize: key.s(key.kind === "enter" ? 20 : key.kind === "op" ? 30 : 24)
        font.weight: key.kind === "enter" ? Font.DemiBold : key.kind === "op" ? Font.Light : Font.Normal
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: { key.action(); keys.forceActiveFocus() }
    }
}
