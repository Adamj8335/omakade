import QtQuick
import QtQuick.Layouts

FocusScope {
    id: root

    property string value: ""
    property string title: "SEARCH YOUR LIBRARY"
    property string placeholder: "Start typing"
    property bool passwordMode: false
    property int maximumLength: 128
    property string keyboardMode: "upper"
    property string gridObjectName: "couchKeyboardGrid"
    readonly property int columns: 10
    readonly property real uiScale: Math.max(1, Math.min(2,
                                                         Math.min(width / 1920,
                                                                  height / 1080)))
    readonly property var upperKeys: [
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
        "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
        "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3",
        "4", "5", "6", "7", "8", "9", ".", "-", "_", "@",
        "BACKSPACE", "SPACE", "CLEAR", "SHIFT", "SYMBOLS", "DONE"
    ]
    readonly property var lowerKeys: [
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
        "k", "l", "m", "n", "o", "p", "q", "r", "s", "t",
        "u", "v", "w", "x", "y", "z", "0", "1", "2", "3",
        "4", "5", "6", "7", "8", "9", ".", "-", "_", "@",
        "BACKSPACE", "SPACE", "CLEAR", "SHIFT", "SYMBOLS", "DONE"
    ]
    readonly property var symbolKeys: [
        "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*",
        "+", ",", "-", ".", "/", ":", ";", "<", "=", ">",
        "?", "@", "[", "]", "^", "_", "{", "|", "}", "~",
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "BACKSPACE", "SPACE", "CLEAR", "LETTERS", "SHIFT", "DONE"
    ]
    readonly property var keys: keyboardMode === "symbols" ? symbolKeys
                                : keyboardMode === "lower" ? lowerKeys : upperKeys

    signal valueEdited(string value)
    signal accepted(string value)
    signal canceled()

    Accessible.name: "On-screen keyboard"
    Accessible.role: Accessible.Pane

    function alpha(color, amount) {
        return Qt.rgba(color.r, color.g, color.b, amount)
    }

    function focusKeyboard() {
        keyGrid.currentIndex = 0
        keyGrid.forceActiveFocus(Qt.TabFocusReason)
    }

    function appendText(text) {
        if (text.length > 0 && value.length < maximumLength) {
            value += text.slice(0, maximumLength - value.length)
            valueEdited(value)
        }
    }

    function displayValue() {
        if (!passwordMode) {
            return value
        }
        let masked = ""
        for (let index = 0; index < value.length; ++index) {
            masked += "•"
        }
        return masked
    }

    function activateKey(index) {
        if (index < 0 || index >= keys.length) {
            return
        }
        const key = keys[index]
        if (key === "BACKSPACE") {
            value = value.slice(0, -1)
        } else if (key === "SPACE") {
            appendText(" ")
            return
        } else if (key === "CLEAR") {
            value = ""
        } else if (key === "DONE") {
            accepted(value)
            return
        } else if (key === "SHIFT") {
            keyboardMode = keyboardMode === "lower" ? "upper" : "lower"
        } else if (key === "SYMBOLS") {
            keyboardMode = "symbols"
        } else if (key === "LETTERS") {
            keyboardMode = "upper"
        } else {
            appendText(key)
            return
        }
        valueEdited(value)
    }

    Keys.priority: Keys.AfterItem
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Backspace) {
            root.value = root.value.slice(0, -1)
            root.valueEdited(root.value)
            event.accepted = true
        } else if (event.text.length > 0 && event.text.charCodeAt(0) >= 32) {
            root.appendText(event.text)
            event.accepted = true
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.alpha(Theme.darkerBackground, 0.92)

        MouseArea { anchors.fill: parent }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 96 * root.uiScale, 1180 * root.uiScale)
        height: Math.min(parent.height - 96 * root.uiScale, 760 * root.uiScale)
        radius: Math.max(14 * root.uiScale, Theme.cornerRadius * 2)
        color: root.alpha(Theme.background, 0.98)
        border.width: 1
        border.color: root.alpha(Theme.foreground, 0.18)

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 42 * root.uiScale
            spacing: 22 * root.uiScale

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3 * root.uiScale

                    Text {
                        text: root.title
                        color: Theme.brightForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: 26 * root.uiScale
                        font.weight: Font.Bold
                    }
                    Text {
                        text: "Use the directional pad and confirm button."
                        color: Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 13 * root.uiScale
                    }
                }

                GlassButton {
                    text: "CANCEL"
                    onClicked: root.canceled()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72 * root.uiScale
                radius: Math.max(8 * root.uiScale, Theme.cornerRadius)
                color: root.alpha(Theme.foreground, 0.07)
                border.width: 2
                border.color: Theme.accent

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 22 * root.uiScale
                    anchors.rightMargin: 22 * root.uiScale
                    verticalAlignment: Text.AlignVCenter
                    text: root.value.length > 0
                          ? root.displayValue()
                          : root.placeholder
                    textFormat: Text.PlainText
                    color: root.value.length > 0 ? Theme.brightForeground : Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 23 * root.uiScale
                    elide: Text.ElideRight
                }
            }

            GridView {
                id: keyGrid
                objectName: root.gridObjectName
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: root.keys
                cellWidth: width / root.columns
                cellHeight: height / Math.ceil(root.keys.length / root.columns)
                currentIndex: 0
                clip: true
                keyNavigationEnabled: false

                Keys.onLeftPressed: function(event) {
                    if (currentIndex % root.columns > 0) {
                        currentIndex--
                    }
                    event.accepted = true
                }
                Keys.onRightPressed: function(event) {
                    if (currentIndex % root.columns < root.columns - 1
                            && currentIndex + 1 < count) {
                        currentIndex++
                    }
                    event.accepted = true
                }
                Keys.onUpPressed: function(event) {
                    if (currentIndex >= root.columns) {
                        currentIndex -= root.columns
                    }
                    event.accepted = true
                }
                Keys.onDownPressed: function(event) {
                    if (currentIndex + root.columns < count) {
                        currentIndex += root.columns
                    }
                    event.accepted = true
                }
                Keys.onReturnPressed: function(event) {
                    root.activateKey(currentIndex)
                    event.accepted = true
                }
                Keys.onEnterPressed: function(event) {
                    root.activateKey(currentIndex)
                    event.accepted = true
                }
                Keys.onEscapePressed: function(event) {
                    root.canceled()
                    event.accepted = true
                }

                delegate: Rectangle {
                    required property int index
                    required property string modelData

                    width: keyGrid.cellWidth - 10 * root.uiScale
                    height: keyGrid.cellHeight - 10 * root.uiScale
                    x: 5 * root.uiScale
                    y: 5 * root.uiScale
                    radius: Math.max(7 * root.uiScale, Theme.cornerRadius)
                    color: keyMouse.pressed
                           ? root.alpha(Theme.accent, 0.28)
                           : keyGrid.currentIndex === index
                             ? root.alpha(Theme.accent, 0.18)
                             : root.alpha(Theme.foreground, 0.055)
                    border.width: keyGrid.currentIndex === index ? 3 : 1
                    border.color: keyGrid.currentIndex === index
                                  ? Theme.accent
                                  : root.alpha(Theme.foreground, 0.16)

                    Text {
                        anchors.centerIn: parent
                        text: modelData === "BACKSPACE" ? "DELETE"
                              : modelData === "SPACE" ? "SPACE"
                              : modelData
                        color: Theme.brightForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: (modelData.length > 1 ? 13 : 22) * root.uiScale
                        font.weight: Font.DemiBold
                    }

                    MouseArea {
                        id: keyMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            keyGrid.currentIndex = index
                            root.activateKey(index)
                            keyGrid.forceActiveFocus(Qt.MouseFocusReason)
                        }
                    }
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight
                spacing: 18 * root.uiScale

                Text {
                    text: Controller.primaryGlyph + "  TYPE"
                    color: Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 12 * root.uiScale
                    font.weight: Font.DemiBold
                }
                Text {
                    text: Controller.backGlyph + "  CANCEL"
                    color: Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 12 * root.uiScale
                    font.weight: Font.DemiBold
                }
            }
        }
    }
}
