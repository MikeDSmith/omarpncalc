import QtQuick
import QtQuick.Layouts
import QtQuick.Window

// Keys: 0-9 . e (exponent)   Enter = enter (push / dup)   + - * /
//       Backspace = delete digit (or drop X)   Delete or d = drop
//       x or Tab = swap   r = roll down   R = roll up   s = +/-
//       i = 1/x   q = sqrt   m = mr   M = m+   Ctrl+M = m−
//       c = c (zero X), press again for ac (clear stack and memory)
//       Escape = clear all, then close when already empty   Ctrl+W = close
//       Ctrl+C / Super+C = copy X   Ctrl+V / Super+V = paste as X
Window {
    id: win
    title: "OmaRPNCalc"
    width: designWidth
    height: designHeight
    minimumWidth: 320
    minimumHeight: 520
    visible: true
    color: theme.background

    // Every size in the face is expressed at the design size below and scales
    // with the window, so resizing by hand (Super+= / Super+- on Hyprland)
    // scales the whole face.
    //
    // The desktop text scale is deliberately NOT followed. That knob asks for
    // bigger text, but font sizes here come from the window size, so honouring
    // it meant growing the whole window -- padding, gaps, key heights and all
    // -- which made a calculator you summon and dismiss take over the screen.
    // The size below is the one worth having at any text setting.
    readonly property int designWidth: 400
    readonly property int designHeight: 660
    readonly property real uiScale: Math.min(width / designWidth, height / designHeight)

    // The one thing that does change the size: a screen too small to hold it.
    // Wayland never hands a client the work area (Qt's Screen.desktopAvailable*
    // just repeats the full screen size), so Hyprland is asked for it.
    Component.onCompleted: {
        var area = hyprland.workArea()
        if (!(area.width > 0 && area.height > 0))
            area = Qt.size(Screen.desktopAvailableWidth, Screen.desktopAvailableHeight)
        if (!(area.width > 0 && area.height > 0))
            return

        var scale = Math.min(1, area.width / designWidth, area.height / designHeight)
        if (scale < 1) {
            width = Math.round(designWidth * scale)
            height = Math.round(designHeight * scale)
        }
    }

    // Scale a design-size length to the current window.
    function s(px) { return Math.max(1, Math.round(px * uiScale)) }
    function alpha(c, a) { return Qt.rgba(c.r, c.g, c.b, a) }

    property int pad: s(20)
    property int gap: s(12)

    Item {
        id: keys
        anchors.fill: parent
        focus: true

        Keys.onPressed: function (event) {
            var mod = event.modifiers & (Qt.ControlModifier | Qt.MetaModifier)
            if (mod) {
                if (event.key === Qt.Key_C) calc.copy()
                else if (event.key === Qt.Key_V) calc.paste()
                else if (event.key === Qt.Key_M) calc.memoryAdd(-1)
                else if (event.key === Qt.Key_W || event.key === Qt.Key_Q) win.close()
                else return
                event.accepted = true
                return
            }
            var t = event.text
            if (t.length === 1 && t >= "0" && t <= "9") calc.digit(t)
            else if (t === "." || t === ",") calc.digit(".")
            else if (t === "e" || t === "E") calc.digit("e")
            else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || t === "=") calc.enter()
            else if (t === "+") calc.add()
            else if (t === "-") calc.subtract()
            else if (t === "*") calc.multiply()
            else if (t === "/") calc.divide()
            else if (event.key === Qt.Key_Backspace) calc.backspace()
            else if (event.key === Qt.Key_Delete || t === "d") calc.drop()
            else if (event.key === Qt.Key_Tab || t === "x") calc.swap()
            else if (t === "r") calc.rollDown()
            else if (t === "R") calc.rollUp()
            else if (t === "s") calc.negate()
            else if (t === "i") calc.reciprocal()
            else if (t === "q") calc.sqrt()
            else if (t === "m") calc.memoryRecall()
            else if (t === "M") calc.memoryAdd(1)
            else if (t === "c" || t === "C") calc.clearKey()
            else if (event.key === Qt.Key_Escape) {
                if (calc.empty) win.close()
                else calc.clearAll()
            }
            else return
            event.accepted = true
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: win.pad
            spacing: win.gap

            // ---------- stack display ----------
            Column {
                Layout.fillWidth: true
                Layout.preferredHeight: win.s(150)
                spacing: win.s(2)

                Repeater {
                    model: calc.levels
                    delegate: Item {
                        required property var modelData
                        readonly property bool isX: modelData.level === 1
                        width: parent.width
                        height: win.s(isX ? 56 : 28)

                        Text {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            // The X label doubles as the status line for errors and "Copied".
                            readonly property bool status: parent.isX && calc.message !== ""
                            text: status ? calc.message : modelData.level + ":"
                            color: status ? (calc.message === "Copied" ? theme.accent : theme.urgent)
                                          : win.alpha(theme.foreground, 0.35)
                            font.family: "monospace"
                            font.pixelSize: win.s(status ? 14 : parent.isX ? 18 : 14)
                        }

                        Row {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: win.s(2)
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.value
                                color: parent.parent.isX ? theme.foreground : win.alpha(theme.foreground, 0.55)
                                font.family: "monospace"
                                font.pixelSize: win.s(parent.parent.isX ? 44 : 22)
                                font.weight: parent.parent.isX ? Font.Medium : Font.Normal
                                // Never hide digits: shrink the font until the whole
                                // number fits (the engine bounds the width, so it always does).
                                fontSizeMode: Text.HorizontalFit
                                minimumPixelSize: win.s(parent.parent.isX ? 20 : 12)
                                horizontalAlignment: Text.AlignRight
                                width: Math.min(implicitWidth, keys.width - win.pad * 2 - win.s(44))
                            }
                            // Insertion bar while a number is being typed.
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                visible: modelData.typing
                                width: win.s(3)
                                height: win.s(parent.parent.isX ? 36 : 18)
                                radius: 1
                                color: theme.accent
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: win.alpha(theme.foreground, 0.12)
            }

            // ---------- keypad ----------
            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 4
                columnSpacing: win.gap
                rowSpacing: win.gap

                Key { label: calc.clearArmed ? "ac" : "c"; action: calc.clearKey }
                Key { label: "drop"; action: calc.drop }
                Key { label: "swap"; action: calc.swap }
                Key { label: "roll"; action: calc.rollDown }

                // The right column mirrors omacalc: ÷ × − + then enter where = sits.
                Key { label: "±";   action: calc.negate }
                Key { label: "1/x"; action: calc.reciprocal }
                Key { label: "√x";  action: calc.sqrt }
                Key { label: "÷"; kind: "op"; action: calc.divide }

                Key { label: "mr"; action: calc.memoryRecall
                      labelColor: calc.memoryHeld ? theme.accent : win.alpha(theme.foreground, 0.45) }
                Key { label: "m+"; action: function () { calc.memoryAdd(1) } }
                Key { label: "m−"; action: function () { calc.memoryAdd(-1) } }
                Key { label: "×"; kind: "op"; action: calc.multiply }

                Key { label: "7"; action: function () { calc.digit("7") } }
                Key { label: "8"; action: function () { calc.digit("8") } }
                Key { label: "9"; action: function () { calc.digit("9") } }
                Key { label: "−"; kind: "op"; action: calc.subtract }

                Key { label: "4"; action: function () { calc.digit("4") } }
                Key { label: "5"; action: function () { calc.digit("5") } }
                Key { label: "6"; action: function () { calc.digit("6") } }
                Key { label: "+"; kind: "op"; action: calc.add }

                Key { label: "1"; action: function () { calc.digit("1") } }
                Key { label: "2"; action: function () { calc.digit("2") } }
                Key { label: "3"; action: function () { calc.digit("3") } }
                Key { label: "enter"; kind: "enter"; Layout.rowSpan: 2; action: calc.enter }

                Key { label: "0"; action: function () { calc.digit("0") } }
                Key { label: "."; action: function () { calc.digit(".") } }
                Key { icon: "backspace"; action: calc.backspace }
            }
        }
    }
}
