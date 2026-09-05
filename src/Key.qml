import QtQuick
import QtQuick.Layouts
import QtQuick.Window

// One keypad button. Digits sit almost flush with the page, the operator
// column lifts a step lighter, and enter fills with the accent colour.
Rectangle {
    id: key

    property string label
    // "backspace" draws the erase icon in place of the label.
    property string icon: ""
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
        visible: key.icon === ""
        text: key.label
        color: key.labelColor
        font.family: "monospace"
        font.pixelSize: key.s(key.kind === "enter" ? 20 : key.kind === "op" ? 30 : 24)
        font.weight: key.kind === "enter" ? Font.DemiBold : key.kind === "op" ? Font.Light : Font.Normal
    }

    // Drawn rather than set as the character ⌫ (U+232B): not every monospace
    // font carries that glyph -- iA Writer Mono S does not -- so whichever font
    // `omarchy font set` picks could silently fall back to a different typeface
    // for this one key. Stroking it also keeps its weight with the digits
    // instead of the filled block most fonts draw.
    Canvas {
        id: eraseIcon
        anchors.centerIn: parent
        visible: key.icon === "backspace"
        // 1.3x the digit size, so it reads as one of the labels.
        width: key.s(31)
        height: Math.round(width * 0.72)

        onPaint: {
            var ctx = getContext("2d")
            var w = width
            var h = height
            var notch = w * 0.28
            ctx.clearRect(0, 0, w, h)
            ctx.strokeStyle = String(key.labelColor)
            ctx.lineWidth = Math.max(1.4, w * 0.07)
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            // The key cap: a rectangle whose left edge tapers to a point.
            ctx.beginPath()
            ctx.moveTo(notch, 1)
            ctx.lineTo(w - 1, 1)
            ctx.lineTo(w - 1, h - 1)
            ctx.lineTo(notch, h - 1)
            ctx.lineTo(1, h / 2)
            ctx.closePath()
            ctx.stroke()

            // The x inside.
            var cx = notch + (w - notch) / 2
            var cy = h / 2
            var arm = h * 0.18
            ctx.beginPath()
            ctx.moveTo(cx - arm, cy - arm)
            ctx.lineTo(cx + arm, cy + arm)
            ctx.moveTo(cx + arm, cy - arm)
            ctx.lineTo(cx - arm, cy + arm)
            ctx.stroke()
        }

        // A Canvas does not repaint itself when what it draws from changes.
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Connections {
            target: key
            function onLabelColorChanged() { eraseIcon.requestPaint() }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: { key.action(); keys.forceActiveFocus() }
    }
}
