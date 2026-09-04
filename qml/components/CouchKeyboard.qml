import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    property string value: ""
    readonly property int columns: 10
    readonly property var keys: [
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
        "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
        "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3",
        "4", "5", "6", "7", "8", "9", "BACKSPACE", "SPACE", "CLEAR", "DONE"
    ]

    signal valueEdited(string value)
    signal accepted(string value)
    signal canceled()

    Accessible.name: "On-screen keyboard"
    Accessible.role: Accessible.Keyboard

    function alpha(color, amount) {
        return Qt.rgba(color.r, color.g, color.b, amount)
    }

    function focusKeyboard() {
        keyGrid.currentIndex = 0
        keyGrid.forceActiveFocus(Qt.TabFocusReason)
    }

    function activateKey(index) {
        if (index < 0 || index >= keys.length) {
            return
        }
        const key = keys[index]
        if (key === "BACKSPACE") {
            value = value.slice(0, -1)
        } else if (key === "SPACE") {
            value += " "
        } else if (key === "CLEAR") {
            value = ""
        } else if (key === "DONE") {
            accepted(value)
            return
        } else if (value.length < 80) {
            value += key
        }
        valueEdited(value)
    }

    Rectangle {
        anchors.fill: parent
        color: root.alpha(Theme.darkerBackground, 0.92)

        MouseArea { anchors.fill: parent }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 96, 1180)
        height: Math.min(parent.height - 96, 760)
        radius: Math.max(14, Theme.cornerRadius * 2)
        color: root.alpha(Theme.background, 0.98)
        border.width: 1
        border.color: root.alpha(Theme.foreground, 0.18)

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 42
            spacing: 22

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    Text {
                        text: "SEARCH YOUR LIBRARY"
                        color: Theme.brightForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: 26
                        font.weight: Font.Bold
                    }
                    Text {
                        text: "Use the directional pad and confirm button."
                        color: Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 13
                    }
                }

                GlassButton {
                    text: "CANCEL"
                    onClicked: root.canceled()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72
                radius: Math.max(8, Theme.cornerRadius)
                color: root.alpha(Theme.foreground, 0.07)
                border.width: 2
                border.color: Theme.accent

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 22
                    anchors.rightMargin: 22
                    verticalAlignment: Text.AlignVCenter
                    text: root.value.length > 0 ? root.value : "Start typing"
                    textFormat: Text.PlainText
                    color: root.value.length > 0 ? Theme.brightForeground : Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 23
                    elide: Text.ElideRight
                }
            }

            GridView {
                id: keyGrid
                objectName: "couchKeyboardGrid"
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: root.keys
                cellWidth: width / root.columns
                cellHeight: height / 4
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

                    width: keyGrid.cellWidth - 10
                    height: keyGrid.cellHeight - 10
                    x: 5
                    y: 5
                    radius: Math.max(7, Theme.cornerRadius)
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
                        font.pixelSize: modelData.length > 1 ? 13 : 22
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
                spacing: 18

                Text {
                    text: Controller.primaryGlyph + "  TYPE"
                    color: Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                Text {
                    text: Controller.backGlyph + "  CANCEL"
                    color: Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
            }
        }
    }
}
