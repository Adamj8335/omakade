import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import "../components"

Item {
    id: root

    Accessible.name: (game.title || "Game") + " details"
    Accessible.role: Accessible.Pane

    required property var game
    signal backRequested()
    signal favoriteRequested()
    signal playRequested()
    signal manageRequested()
    signal hiddenRequested()

    function alpha(color, value) {
        return Qt.rgba(color.r, color.g, color.b, value)
    }

    Keys.onEscapePressed: function(event) {
        root.backRequested()
        event.accepted = true
    }

    Rectangle {
        anchors.fill: parent
        color: root.alpha(Theme.darkerBackground, 0.92)
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.min(parent.height * 0.58, 500)
        opacity: 0.42
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: root.game.accentStart || Theme.accent }
            GradientStop { position: 1.0; color: root.game.accentEnd || Theme.blue }
        }
    }

    Image {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.min(parent.height * 0.58, 500)
        source: root.game.heroPath || ""
        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectCrop
        sourceSize.width: Math.ceil(width * Math.max(1, Screen.devicePixelRatio))
        sourceSize.height: Math.ceil(height * Math.max(1, Screen.devicePixelRatio))
        opacity: status === Image.Ready ? 0.48 : 0
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.min(parent.height * 0.62, 540)
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: Theme.darkerBackground }
        }
    }

    GlassButton {
        id: backButton
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 24
        text: "BACK"
        iconText: "←"
        compact: true
        onClicked: root.backRequested()
    }

    ScrollView {
        anchors.fill: parent
        anchors.topMargin: 80
        anchors.leftMargin: Math.max(28, parent.width * 0.055)
        anchors.rightMargin: Math.max(28, parent.width * 0.055)
        anchors.bottomMargin: 22
        contentWidth: availableWidth
        clip: true

        RowLayout {
            width: parent.width
            spacing: Math.max(28, width * 0.045)

            Rectangle {
                Layout.preferredWidth: Math.min(330, root.width * 0.28)
                Layout.preferredHeight: Layout.preferredWidth * 1.42
                Layout.alignment: Qt.AlignTop
                radius: Math.max(6, Theme.cornerRadius)
                clip: true
                border.color: root.alpha(Theme.foreground, 0.22)
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: root.game.accentStart || Theme.accent }
                    GradientStop { position: 1.0; color: root.game.accentEnd || Theme.blue }
                }

                Image {
                    id: coverArtwork
                    anchors.fill: parent
                    source: root.game.coverPath || ""
                    asynchronous: true
                    cache: false
                    fillMode: Image.PreserveAspectCrop
                    sourceSize.width: Math.ceil(width * Math.max(1, Screen.devicePixelRatio))
                    sourceSize.height: Math.ceil(height * Math.max(1, Screen.devicePixelRatio))
                    opacity: status === Image.Ready ? 1 : 0
                }

                Rectangle {
                    visible: coverArtwork.status !== Image.Ready
                    width: parent.width * 0.95
                    height: width
                    radius: width / 2
                    x: parent.width * 0.44
                    y: -height * 0.18
                    color: root.alpha(Theme.brightForeground, 0.10)
                }

                Text {
                    visible: coverArtwork.status !== Image.Ready
                    anchors.centerIn: parent
                    text: root.game.coverMark || "◇"
                    color: root.alpha(Theme.brightForeground, 0.9)
                    font.family: Theme.fontFamily
                    font.pixelSize: Math.max(74, parent.width * 0.34)
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.height * 0.34
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 1.0; color: root.alpha(Theme.darkerBackground, 0.84) }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 16

                Text {
                    Layout.fillWidth: true
                    text: root.game.title || "Unknown game"
                    color: Theme.brightForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Math.max(28, Math.min(54, root.width * 0.045))
                    font.weight: Font.Bold
                    wrapMode: Text.Wrap
                }

                RowLayout {
                    spacing: 10
                    Text {
                        text: (root.game.subtitle || "GAME") .toUpperCase()
                        color: Theme.accent
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                    Text {
                        text: "·"
                        color: root.alpha(Theme.foreground, 0.4)
                    }
                    Text {
                        text: root.game.year || ""
                        color: Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 720
                    text: root.game.description || ""
                    color: Theme.foreground
                    opacity: 0.84
                    font.family: Theme.fontFamily
                    font.pixelSize: 13
                    lineHeight: 1.45
                    wrapMode: Text.Wrap
                }

                RowLayout {
                    spacing: 10

                    GlassButton {
                        id: playButton
                        text: "PLAY"
                        iconText: "▶"
                        primary: true
                        onClicked: root.playRequested()
                        Component.onCompleted: forceActiveFocus()
                    }

                    GlassButton {
                        text: root.game.favorite ? "FAVORITE" : "ADD FAVORITE"
                        iconText: root.game.favorite ? "♥" : "♡"
                        onClicked: root.favoriteRequested()
                    }

                    GlassButton {
                        visible: root.game.source === "Steam"
                        text: "MANAGE IN STEAM"
                        onClicked: root.manageRequested()
                    }

                    GlassButton {
                        visible: root.game.source === "Steam"
                        text: root.game.hidden ? "UNHIDE" : "HIDE"
                        onClicked: root.hiddenRequested()
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 12
                    columns: root.width < 1050 ? 1 : 3
                    columnSpacing: 10
                    rowSpacing: 10

                    Repeater {
                        model: [
                            { label: "PLAYTIME", value: (root.game.hours || 0) + " HOURS" },
                            { label: "ACHIEVEMENTS", value: (root.game.achievementsUnlocked || 0) + " / " + (root.game.achievementsTotal || 0) },
                            { label: "COMPLETION", value: (root.game.progress || 0) + "%" }
                        ]

                        Rectangle {
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.minimumWidth: 150
                            Layout.preferredHeight: 88
                            radius: Math.max(5, Theme.cornerRadius)
                            color: root.alpha(Theme.foreground, 0.045)
                            border.color: root.alpha(Theme.foreground, 0.13)

                            Column {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.leftMargin: 16
                                spacing: 7
                                Text {
                                    text: modelData.label
                                    color: Theme.mutedText
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 9
                                    font.weight: Font.DemiBold
                                }
                                Text {
                                    text: modelData.value
                                    color: Theme.brightForeground
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 16
                                    font.weight: Font.DemiBold
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 12
                    spacing: 9

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "ACHIEVEMENT PROGRESS"
                            color: Theme.foreground
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: (root.game.progress || 0) + "%"
                            color: Theme.accent
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 5
                        radius: 3
                        color: root.alpha(Theme.foreground, 0.1)

                        Rectangle {
                            width: parent.width * (root.game.progress || 0) / 100
                            height: parent.height
                            radius: parent.radius
                            color: Theme.accent
                        }
                    }
                }
            }
        }
    }
}
