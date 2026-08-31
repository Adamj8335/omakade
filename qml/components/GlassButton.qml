import QtQuick
import QtQuick.Controls

Button {
    id: root

    property string iconText: ""
    property bool primary: false
    property bool selected: false
    property bool compact: false

    function alpha(color, value) {
        return Qt.rgba(color.r, color.g, color.b, value)
    }

    implicitHeight: compact ? 34 : 42
    implicitWidth: Math.max(compact ? 76 : 104, contentRow.implicitWidth + (compact ? 22 : 30))
    leftPadding: compact ? 11 : 15
    rightPadding: leftPadding
    spacing: 8
    focusPolicy: Qt.StrongFocus

    background: Rectangle {
        radius: Math.max(4, Theme.cornerRadius)
        color: root.down
               ? root.alpha(root.primary ? Theme.accent : Theme.foreground, 0.24)
               : root.hovered || root.activeFocus
                 ? root.alpha(root.primary ? Theme.accent : Theme.foreground, 0.14)
                 : root.primary
                   ? root.alpha(Theme.accent, 0.18)
                   : root.selected
                     ? root.alpha(Theme.foreground, 0.12)
                     : root.alpha(Theme.foreground, 0.045)
        border.width: root.activeFocus ? 2 : 1
        border.color: root.activeFocus
                      ? Theme.accent
                      : root.primary
                        ? root.alpha(Theme.accent, 0.58)
                        : root.alpha(Theme.foreground, root.hovered ? 0.32 : 0.16)

        Behavior on color {
            enabled: !Preferences.reducedMotion
            ColorAnimation { duration: 120 }
        }
        Behavior on border.color {
            enabled: !Preferences.reducedMotion
            ColorAnimation { duration: 120 }
        }
    }

    contentItem: Row {
        id: contentRow
        spacing: root.spacing
        anchors.centerIn: parent

        Text {
            visible: root.iconText.length > 0
            text: root.iconText
            color: root.enabled ? (root.primary ? Theme.brightForeground : Theme.foreground)
                                : root.alpha(Theme.foreground, 0.35)
            font.family: Theme.fontFamily
            font.pixelSize: root.compact ? 12 : 14
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.text
            color: root.enabled ? (root.primary ? Theme.brightForeground : Theme.foreground)
                                : root.alpha(Theme.foreground, 0.35)
            font.family: Theme.fontFamily
            font.pixelSize: root.compact ? 11 : 12
            font.weight: root.primary || root.selected ? Font.DemiBold : Font.Medium
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
