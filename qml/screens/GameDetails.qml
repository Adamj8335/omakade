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
    required property var installations
    required property var selectedInstallation
    signal backRequested()
    signal favoriteRequested()
    signal playRequested()
    signal manageRequested()
    signal hiddenRequested()
    signal connectRequested()
    signal coverRequested()
    signal coverResetRequested()
    signal installationSelected(var installation)
    signal linkRequested()
    signal unlinkRequested()

    function alpha(color, value) {
        return Qt.rgba(color.r, color.g, color.b, value)
    }

    function insightValue(value) {
        return value > 0 ? value + " H" : "NO DATA"
    }

    Keys.onEscapePressed: function(event) {
        root.backRequested()
        event.accepted = true
    }

    Rectangle {
        anchors.fill: parent
        color: root.alpha(Theme.darkerBackground, 0.76)
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

            ColumnLayout {
                Layout.preferredWidth: Math.min(330, root.width * 0.28)
                Layout.alignment: Qt.AlignTop
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width * 1.5
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

                RowLayout {
                    Layout.fillWidth: true
                    visible: !DemoMode
                    spacing: 8
                    GlassButton {
                        Layout.fillWidth: true
                        compact: true
                        text: "CHANGE COVER"
                        onClicked: root.coverRequested()
                    }
                    GlassButton {
                        visible: root.game.customCover || false
                        compact: true
                        text: "RESET"
                        onClicked: root.coverResetRequested()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: !DemoMode
                    spacing: 8
                    GlassButton {
                        Layout.fillWidth: true
                        compact: true
                        text: root.game.linked ? "UNLINK INSTALLATIONS" : "LINK INSTALLATION"
                        onClicked: root.game.linked ? root.unlinkRequested() : root.linkRequested()
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
                        text: (root.game.linked
                               ? root.game.linkedSources
                               : (root.game.subtitle || "GAME")).toUpperCase()
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

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.installations.length > 1
                    spacing: 7
                    Text {
                        text: "LAUNCH WITH"
                        color: Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 9
                        font.weight: Font.DemiBold
                    }
                    RowLayout {
                        spacing: 8
                        Repeater {
                            model: root.installations
                            GlassButton {
                                required property var modelData
                                compact: true
                                text: (modelData.source || "LOCAL").toUpperCase()
                                      + (modelData.runner ? " · " + modelData.runner.toUpperCase() : "")
                                selected: root.selectedInstallation.source === modelData.source
                                          && (root.selectedInstallation.runner || "") === (modelData.runner || "")
                                          && root.selectedInstallation.appId === modelData.appId
                                onClicked: root.installationSelected(modelData)
                            }
                        }
                    }
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
                        visible: root.selectedInstallation.source === "Steam" || root.selectedInstallation.source === "Lutris" || root.selectedInstallation.source === "Heroic"
                        text: "MANAGE IN " + (root.selectedInstallation.source || "LAUNCHER").toUpperCase()
                        onClicked: root.manageRequested()
                    }

                    GlassButton {
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
                        model: root.selectedInstallation.source === "Steam"
                               ? [
                                   { label: "PLAYTIME", value: (root.game.hours || 0) + " HOURS" },
                                   { label: "ACHIEVEMENTS", value: (Achievements.unlocked || root.game.achievementsUnlocked || 0) + " / " + (Achievements.total || root.game.achievementsTotal || 0) },
                                   { label: "COMPLETION", value: Achievements.total > 0 ? Math.round(Achievements.unlocked * 100 / Achievements.total) + "%" : (root.game.progress || 0) + "%" }
                               ]
                               : [
                                   { label: "PLAYTIME", value: (root.game.hours || 0) + " HOURS" },
                                   { label: "SOURCE", value: (root.selectedInstallation.source || "LOCAL").toUpperCase() },
                                   { label: "LAUNCHER", value: (root.selectedInstallation.subtitle || root.selectedInstallation.source || "LOCAL").toUpperCase() }
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
                    spacing: 10
                    visible: root.selectedInstallation.source === "Steam" && Insights !== null

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "GAME INSIGHTS · IGDB"
                            color: Theme.brightForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            font.letterSpacing: 0.6
                        }
                        Item { Layout.fillWidth: true }
                        GlassButton {
                            compact: true
                            text: Insights && Insights.configured
                                  ? (Insights.busy ? "REFRESHING" : "REFRESH")
                                  : "CONNECT IGDB"
                            enabled: Insights && !Insights.busy
                            onClicked: {
                                if (Insights.configured) {
                                    Insights.refreshSteam(root.game.appId)
                                } else {
                                    root.connectRequested()
                                }
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: Insights ? Insights.statusText : ""
                        color: Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        wrapMode: Text.Wrap
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        visible: Insights ? Insights.available : false
                        columns: root.width < 1120 ? 2 : 4
                        columnSpacing: 10
                        rowSpacing: 10

                        Repeater {
                            model: Insights ? [
                                { label: "IGDB CRITIC", value: Insights.criticScore >= 0 ? Insights.criticScore + " / 100" : "NO SCORE" },
                                { label: "RUSHED", value: root.insightValue(Insights.rushedHours) },
                                { label: "MAIN + EXTRAS", value: root.insightValue(Insights.normalHours) },
                                { label: "COMPLETIONIST", value: root.insightValue(Insights.completeHours) }
                            ] : []

                            Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.minimumWidth: 130
                                Layout.preferredHeight: 72
                                radius: Math.max(5, Theme.cornerRadius)
                                color: root.alpha(Theme.foreground, 0.045)
                                border.color: root.alpha(Theme.foreground, 0.13)

                                Column {
                                    anchors.left: parent.left
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.leftMargin: 14
                                    spacing: 6
                                    Text {
                                        text: modelData.label
                                        color: Theme.mutedText
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 8
                                        font.weight: Font.DemiBold
                                    }
                                    Text {
                                        text: modelData.value
                                        color: Theme.brightForeground
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                    }
                                }
                            }
                        }
                    }

                    Text {
                        visible: Insights ? Insights.available : false
                        text: "Critic aggregate and time estimates provided by IGDB"
                        color: root.alpha(Theme.foreground, 0.48)
                        font.family: Theme.fontFamily
                        font.pixelSize: 8
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 12
                    spacing: 9
                    visible: root.selectedInstallation.source === "Steam"

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
                            text: Achievements.total > 0 ? Math.round(Achievements.unlocked * 100 / Achievements.total) + "%" : (root.game.progress || 0) + "%"
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
                            width: parent.width * (Achievements.total > 0
                                                   ? Achievements.unlocked / Achievements.total
                                                   : (root.game.progress || 0) / 100)
                            height: parent.height
                            radius: parent.radius
                            color: Theme.accent
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 18
                    spacing: 10
                    visible: root.selectedInstallation.source === "Steam"

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "ACHIEVEMENTS"
                            color: Theme.brightForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.weight: Font.Bold
                            font.letterSpacing: 0.7
                        }
                        Item { Layout.fillWidth: true }
                        GlassButton {
                            visible: SteamAccount !== null
                            compact: true
                            text: SteamAccount && SteamAccount.hasApiKey
                                  ? (SteamAccount.busy ? "REFRESHING" : "REFRESH STEAM")
                                  : "CONNECT STEAM"
                            enabled: !SteamAccount || !SteamAccount.busy
                            onClicked: {
                                if (SteamAccount.hasApiKey) {
                                    SteamAccount.refreshAchievements(root.game.appId)
                                } else {
                                    root.connectRequested()
                                }
                            }
                        }
                        Text {
                            text: Achievements.unlocked + " / " + Achievements.total
                            color: Theme.accent
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: Achievements.statusText
                        color: Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        wrapMode: Text.Wrap
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: SteamAccount && SteamAccount.statusText.length > 0
                        text: SteamAccount ? SteamAccount.statusText : ""
                        color: SteamAccount && (SteamAccount.state === "invalid-key"
                                                || SteamAccount.state === "private"
                                                || SteamAccount.state === "rate-limited")
                               ? Theme.yellow : Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        wrapMode: Text.Wrap
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        visible: Achievements.total > 0
                        columns: root.width < 1160 ? 1 : 2
                        columnSpacing: 10
                        rowSpacing: 10

                        Repeater {
                            model: Achievements

                            Rectangle {
                                required property string title
                                required property string description
                                required property string iconPath
                                required property bool unlocked
                                required property double unlockTime
                                required property real rarity
                                required property bool hidden
                                Layout.fillWidth: true
                                Layout.minimumWidth: 260
                                Layout.preferredHeight: 82
                                radius: Math.max(6, Theme.cornerRadius)
                                color: root.alpha(Theme.foreground, unlocked ? 0.075 : 0.035)
                                border.color: unlocked
                                              ? root.alpha(Theme.accent, 0.34)
                                              : root.alpha(Theme.foreground, 0.10)

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 11
                                    spacing: 12

                                    Rectangle {
                                        Layout.preferredWidth: 54
                                        Layout.preferredHeight: 54
                                        radius: 5
                                        color: root.alpha(Theme.darkerBackground, 0.54)
                                        border.color: root.alpha(Theme.foreground, 0.12)
                                        clip: true

                                        Image {
                                            anchors.fill: parent
                                            source: iconPath
                                            asynchronous: true
                                            fillMode: Image.PreserveAspectFit
                                            opacity: unlocked ? 1 : 0.42
                                        }
                                        Text {
                                            visible: iconPath.length === 0
                                            anchors.centerIn: parent
                                            text: unlocked ? "◆" : "◇"
                                            color: unlocked ? Theme.accent : Theme.mutedText
                                            font.pixelSize: 19
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 3
                                        Text {
                                            Layout.fillWidth: true
                                            text: hidden && !unlocked ? "Hidden achievement" : title
                                            color: unlocked ? Theme.brightForeground : Theme.foreground
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 11
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            Layout.fillWidth: true
                                            text: hidden && !unlocked ? "Unlock to reveal details" : description
                                            color: Theme.mutedText
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 9
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            text: (unlocked && unlockTime > 0
                                                   ? "UNLOCKED " + Qt.formatDateTime(new Date(unlockTime * 1000), "MMM d, yyyy").toUpperCase() + "  ·  "
                                                   : "")
                                                  + (rarity > 0 ? rarity.toFixed(1) + "% OF PLAYERS" : "STEAM")
                                            color: unlocked ? Theme.accent : root.alpha(Theme.foreground, 0.45)
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 8
                                            font.weight: Font.DemiBold
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
