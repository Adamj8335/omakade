import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "screens"

ApplicationWindow {
    id: root

    property bool detailOpen: false
    property var selectedGame: ({})
    property int selectedIndex: -1
    property bool smokeReady: false
    property bool diagnosticsOpen: false

    function alpha(color, value) {
        return Qt.rgba(color.r, color.g, color.b, value)
    }

    function openGame(index) {
        selectedIndex = index
        selectedGame = Library.get(index)
        if (!DemoMode && selectedGame.source === "Steam") {
            Achievements.load(selectedGame.appId)
        } else {
            Achievements.load("")
        }
        detailOpen = true
    }

    function closeDetails() {
        detailOpen = false
        Qt.callLater(libraryView.focusGrid)
    }

    function showToast(message) {
        toast.message = message
        toastTimer.restart()
    }

    function playSelected() {
        if (DemoMode) {
            showToast("Demo games cannot be launched")
        } else if (Launcher.launch(selectedGame.source, selectedGame.appId,
                                   selectedGame.flatpak || false)) {
            showToast("Opening " + selectedGame.title + " in " + selectedGame.source)
        } else {
            showToast(Launcher.lastError)
        }
    }

    function manageSelected() {
        if (Launcher.manage(selectedGame.source, selectedGame.appId,
                            selectedGame.flatpak || false)) {
            showToast("Opening " + selectedGame.source)
        } else {
            showToast(Launcher.lastError)
        }
    }

    visible: true
    width: 1380
    height: 880
    minimumWidth: 820
    minimumHeight: 590
    title: "Omakade"
    color: "transparent"

    font.family: Theme.fontFamily

    Shortcut {
        sequence: "Ctrl+F"
        enabled: !root.detailOpen
        onActivated: searchField.forceActiveFocus()
    }
    Shortcut {
        sequence: "F11"
        onActivated: root.visibility = root.visibility === Window.FullScreen
                     ? Window.Windowed : Window.FullScreen
    }
    Shortcut {
        sequence: "Ctrl+M"
        onActivated: {
            Preferences.reducedMotion = !Preferences.reducedMotion
            root.showToast(Preferences.reducedMotion ? "Reduced motion enabled" : "Reduced motion disabled")
        }
    }
    Shortcut {
        sequence: "Ctrl+D"
        onActivated: root.diagnosticsOpen = !root.diagnosticsOpen
    }
    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (root.diagnosticsOpen) {
                root.diagnosticsOpen = false
            } else if (root.detailOpen) {
                root.closeDetails()
            } else if (searchField.text.length > 0) {
                searchField.clear()
                libraryView.focusGrid()
            }
        }
    }
    Shortcut {
        sequence: "Return"
        enabled: !root.detailOpen && !searchField.activeFocus && libraryView.currentIndex >= 0
        onActivated: root.openGame(libraryView.currentIndex)
    }
    Shortcut {
        sequence: "Enter"
        enabled: !root.detailOpen && !searchField.activeFocus && libraryView.currentIndex >= 0
        onActivated: root.openGame(libraryView.currentIndex)
    }
    Shortcut {
        sequence: "Space"
        enabled: !root.detailOpen && !searchField.activeFocus && libraryView.currentIndex >= 0
        onActivated: root.openGame(libraryView.currentIndex)
    }

    onActiveChanged: {
        if (active && !detailOpen && !searchField.activeFocus) {
            Qt.callLater(libraryView.focusGrid)
        }
    }
    onClosing: function(close) {
        close.accepted = true
        Qt.quit()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: root.alpha(Theme.darkerBackground, 0.82) }
            GradientStop { position: 0.48; color: root.alpha(Theme.darkerBackground, 0.68) }
            GradientStop { position: 1.0; color: root.alpha(Theme.darkerBackground, 0.84) }
        }
    }

    Rectangle {
        width: root.width * 0.52
        height: width
        radius: width / 2
        x: root.width * 0.62
        y: -height * 0.62
        color: root.alpha(Theme.accent, 0.10)
    }

    Rectangle {
        width: root.width * 0.42
        height: width
        radius: width / 2
        x: -width * 0.48
        y: root.height * 0.48
        color: root.alpha(Theme.green, 0.055)
    }

    Item {
        id: librarySurface
        anchors.fill: parent
        opacity: root.detailOpen ? 0 : 1
        scale: root.detailOpen ? 0.985 : 1
        visible: opacity > 0
        enabled: !root.detailOpen

        Behavior on opacity {
            enabled: !Preferences.reducedMotion
            NumberAnimation { duration: 150 }
        }
        Behavior on scale {
            enabled: !Preferences.reducedMotion
            NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: Math.max(22, root.width * 0.032)
            anchors.rightMargin: Math.max(22, root.width * 0.032)
            anchors.topMargin: 24
            anchors.bottomMargin: 16
            spacing: 20

            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Row {
                    spacing: 11
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        width: 34
                        height: 34
                        radius: Math.max(7, Theme.cornerRadius)
                        color: root.alpha(Theme.accent, 0.16)
                        border.color: root.alpha(Theme.accent, 0.48)

                        Text {
                            anchors.centerIn: parent
                            text: "◇"
                            color: Theme.accent
                            font.family: Theme.fontFamily
                            font.pixelSize: 19
                            font.weight: Font.DemiBold
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1
                        Text {
                            text: "OMAKADE"
                            color: Theme.brightForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            font.letterSpacing: 1.5
                        }
                        Text {
                            text: Theme.themeName.toUpperCase()
                            color: Theme.mutedText
                            font.family: Theme.fontFamily
                            font.pixelSize: 8
                            font.letterSpacing: 0.7
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                Row {
                    spacing: 5
                    visible: root.width >= 1040

                    GlassButton {
                        text: "ALL"
                        compact: true
                        selected: Library.mode === 0
                        onClicked: {
                            Library.mode = 0
                            libraryView.focusGrid()
                        }
                    }
                    GlassButton {
                        text: "FAVORITES"
                        compact: true
                        selected: Library.mode === 1
                        onClicked: {
                            Library.mode = 1
                            libraryView.focusGrid()
                        }
                    }
                    GlassButton {
                        text: "RECENT"
                        compact: true
                        selected: Library.mode === 2
                        onClicked: {
                            Library.mode = 2
                            libraryView.focusGrid()
                        }
                    }
                    GlassButton {
                        text: "HIDDEN"
                        compact: true
                        visible: !DemoMode
                        selected: Library.mode === 3
                        onClicked: {
                            Library.mode = 3
                            libraryView.focusGrid()
                        }
                    }
                }

                TextField {
                    id: searchField
                    Layout.preferredWidth: Math.min(300, root.width * 0.26)
                    Layout.minimumWidth: 190
                    Layout.preferredHeight: 38
                    placeholderText: "Search games"
                    color: Theme.foreground
                    placeholderTextColor: root.alpha(Theme.foreground, 0.42)
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    leftPadding: 36
                    rightPadding: 12
                    selectByMouse: true
                    focus: false
                    Accessible.name: "Search games"
                    Accessible.description: "Filter the installed game library"

                    onTextChanged: {
                        Library.searchText = text
                        libraryView.currentIndex = Library.rowCount() > 0 ? 0 : -1
                    }
                    Keys.onEscapePressed: function(event) {
                        if (text.length > 0) {
                            clear()
                        }
                        libraryView.focusGrid()
                        event.accepted = true
                    }
                    Keys.onDownPressed: function(event) {
                        libraryView.focusGrid()
                        event.accepted = true
                    }

                    background: Rectangle {
                        radius: Math.max(5, Theme.cornerRadius)
                        color: root.alpha(Theme.foreground, searchField.activeFocus ? 0.075 : 0.045)
                        border.width: searchField.activeFocus ? 2 : 1
                        border.color: searchField.activeFocus
                                      ? Theme.accent
                                      : root.alpha(Theme.foreground, 0.15)
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 13
                        anchors.verticalCenter: parent.verticalCenter
                        text: "⌕"
                        color: searchField.activeFocus ? Theme.accent : Theme.mutedText
                        font.family: Theme.fontFamily
                        font.pixelSize: 15
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: root.width < 1040
                spacing: 6
                GlassButton {
                    text: "ALL"
                    compact: true
                    selected: Library.mode === 0
                    onClicked: Library.mode = 0
                }
                GlassButton {
                    text: "FAVORITES"
                    compact: true
                    selected: Library.mode === 1
                    onClicked: Library.mode = 1
                }
                GlassButton {
                    text: "RECENT"
                    compact: true
                    selected: Library.mode === 2
                    onClicked: Library.mode = 2
                }
                GlassButton {
                    text: "HIDDEN"
                    compact: true
                    visible: !DemoMode
                    selected: Library.mode === 3
                    onClicked: Library.mode = 3
                }
                Item { Layout.fillWidth: true }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Row {
                    spacing: 5
                    visible: !DemoMode
                    GlassButton {
                        text: "ALL SOURCES"
                        compact: true
                        selected: Library.sourceFilter === ""
                        onClicked: {
                            Library.sourceFilter = ""
                            libraryView.currentIndex = Library.rowCount() > 0 ? 0 : -1
                        }
                    }
                    GlassButton {
                        text: "STEAM"
                        compact: true
                        selected: Library.sourceFilter === "Steam"
                        onClicked: {
                            Library.sourceFilter = "Steam"
                            libraryView.currentIndex = Library.rowCount() > 0 ? 0 : -1
                        }
                    }
                    GlassButton {
                        text: "LUTRIS"
                        compact: true
                        selected: Library.sourceFilter === "Lutris"
                        onClicked: {
                            Library.sourceFilter = "Lutris"
                            libraryView.currentIndex = Library.rowCount() > 0 ? 0 : -1
                        }
                    }
                }

                Text {
                    text: Library.mode === 1 ? "FAVORITES" : Library.mode === 2 ? "RECENTLY PLAYED" : Library.mode === 3 ? "HIDDEN" : "YOUR LIBRARY"
                    color: Theme.foreground
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    font.letterSpacing: 0.7
                }
                Text {
                    text: libraryView.count + " GAMES"
                    color: Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 9
                }
                Text {
                    visible: SteamLibrary ? SteamLibrary.scanning : false
                    text: "SYNCING"
                    color: Theme.accent
                    font.family: Theme.fontFamily
                    font.pixelSize: 9
                    font.weight: Font.DemiBold
                }
                Item { Layout.fillWidth: true }
                GlassButton {
                    compact: true
                    text: Library.sortMode === 0 ? "SORT: TITLE" : Library.sortMode === 1 ? "SORT: RECENT" : "SORT: PLAYTIME"
                    onClicked: Library.sortMode = (Library.sortMode + 1) % 3
                }
                Text {
                    text: Controller.connected
                          ? Controller.primaryGlyph + "  OPEN   ·   " + Controller.favoriteGlyph + "  FAVORITE   ·   " + Controller.backGlyph + "  BACK"
                          : "ENTER  OPEN   ·   F  FAVORITE"
                    color: root.alpha(Theme.foreground, 0.42)
                    font.family: Theme.fontFamily
                    font.pixelSize: 8
                    visible: root.width > 930
                }
            }

            LibraryView {
                id: libraryView
                Layout.fillWidth: true
                Layout.fillHeight: true
                libraryModel: Library
                scanning: SteamLibrary ? SteamLibrary.scanning : false
                emptyTitle: Library.sourceFilter === "Lutris" && LutrisLibrary && !LutrisLibrary.lutrisDetected
                            ? "Lutris was not found"
                            : Library.sourceFilter === "Steam" && SteamLibrary && !SteamLibrary.steamDetected
                              ? "Steam was not found"
                              : Library.mode === 3 ? "No hidden games" : "No installed games"
                emptyMessage: Library.sourceFilter === "Lutris" && LutrisLibrary && LutrisLibrary.errorText.length > 0
                              ? LutrisLibrary.errorText
                              : SteamLibrary && SteamLibrary.errorText.length > 0
                                ? SteamLibrary.errorText
                                : "Install a game in Steam or Lutris, then rescan your library."
                onGameActivated: index => root.openGame(index)
                onFavoriteToggled: index => Library.toggleFavorite(index)
                onRefreshRequested: {
                    if (SteamLibrary) {
                        SteamLibrary.refresh()
                    }
                    if (LutrisLibrary) {
                        LutrisLibrary.refresh()
                    }
                }
            }
        }
    }

    Loader {
        id: detailsLoader
        anchors.fill: parent
        active: root.detailOpen
        opacity: root.detailOpen ? 1 : 0
        asynchronous: false

        Behavior on opacity {
            enabled: !Preferences.reducedMotion
            NumberAnimation { duration: 170 }
        }

        sourceComponent: GameDetails {
            game: root.selectedGame
            onBackRequested: root.closeDetails()
            onFavoriteRequested: {
                Library.toggleFavorite(root.selectedIndex)
                root.selectedGame = Library.get(root.selectedIndex)
            }
            onPlayRequested: root.playSelected()
            onManageRequested: root.manageSelected()
            onConnectRequested: root.diagnosticsOpen = true
            onHiddenRequested: {
                Library.toggleHidden(root.selectedIndex)
                root.closeDetails()
            }
        }
    }

    Rectangle {
        id: toast
        property string message: ""
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 26
        width: toastText.implicitWidth + 34
        height: 42
        radius: Math.max(6, Theme.cornerRadius)
        color: root.alpha(Theme.background, 0.94)
        border.color: root.alpha(Theme.accent, 0.5)
        opacity: toastTimer.running ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
            enabled: !Preferences.reducedMotion
            NumberAnimation { duration: 140 }
        }

        Text {
            id: toastText
            anchors.centerIn: parent
            text: toast.message
            color: Theme.foreground
            font.family: Theme.fontFamily
            font.pixelSize: 11
        }
    }

    Timer {
        id: toastTimer
        interval: 2400
    }

    Rectangle {
        anchors.fill: parent
        visible: root.diagnosticsOpen
        z: 20
        color: root.alpha(Theme.darkerBackground, 0.72)

        MouseArea {
            anchors.fill: parent
            onClicked: root.diagnosticsOpen = false
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(610, parent.width - 48)
            height: Math.min(610, parent.height - 48)
            radius: Math.max(8, Theme.cornerRadius)
            color: root.alpha(Theme.background, 0.98)
            border.color: root.alpha(Theme.foreground, 0.2)

            MouseArea { anchors.fill: parent }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 28
                spacing: 14

                Text {
                    text: "DIAGNOSTICS"
                    color: Theme.brightForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: 20
                    font.weight: Font.Bold
                }
                Text {
                    Layout.fillWidth: true
                    text: SteamLibrary ? SteamLibrary.statusText : "Demo library"
                    color: Theme.accent
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
                Text {
                    Layout.fillWidth: true
                    visible: LutrisLibrary !== null
                    text: LutrisLibrary ? LutrisLibrary.statusText : ""
                    color: Theme.accent
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
                Repeater {
                    model: [
                        { label: "LIBRARY", value: libraryView.count + " visible games" },
                        { label: "LOCAL ARTWORK", value: SteamLibrary ? SteamLibrary.artworkCount + " covers" : "Procedural demo art" },
                        { label: "CONTROLLER", value: Controller.connected ? Controller.name : "Not connected" },
                        { label: "DATABASE", value: SteamLibrary ? SteamLibrary.databasePath : "Not used in demo mode" },
                        { label: "ACHIEVEMENT ART", value: (Achievements.cacheBytes / 1048576).toFixed(1) + " MB / " + Preferences.artworkCacheLimitMb + " MB" }
                    ]
                    RowLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        Text {
                            Layout.preferredWidth: 130
                            text: modelData.label
                            color: Theme.mutedText
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                        }
                        Text {
                            Layout.fillWidth: true
                            text: modelData.value
                            color: Theme.foreground
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            elide: Text.ElideMiddle
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: root.alpha(Theme.foreground, 0.12)
                }
                Text {
                    text: "OPTIONAL STEAM CONNECTION"
                    color: Theme.brightForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    text: SteamAccount
                          ? SteamAccount.statusText
                          : "Local Steam data is used in demo mode."
                    color: SteamAccount && (SteamAccount.state === "invalid-key"
                                            || SteamAccount.state === "private"
                                            || SteamAccount.state === "rate-limited")
                           ? Theme.yellow : Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    wrapMode: Text.Wrap
                }
                RowLayout {
                    Layout.fillWidth: true
                    enabled: SteamAccount !== null
                    TextField {
                        id: steamIdField
                        Layout.fillWidth: true
                        placeholderText: "Steam ID"
                        text: SteamAccount ? SteamAccount.steamId : ""
                        color: Theme.foreground
                        placeholderTextColor: root.alpha(Theme.foreground, 0.42)
                        font.family: Theme.fontFamily
                        inputMethodHints: Qt.ImhDigitsOnly
                        background: Rectangle {
                            radius: Math.max(5, Theme.cornerRadius)
                            color: root.alpha(Theme.foreground, 0.045)
                            border.color: root.alpha(Theme.foreground, 0.15)
                        }
                    }
                    GlassButton {
                        compact: true
                        text: "SAVE ID"
                        onClicked: SteamAccount.setSteamId(steamIdField.text)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    enabled: SteamAccount !== null && !SteamAccount.busy
                    TextField {
                        id: apiKeyField
                        Layout.fillWidth: true
                        placeholderText: SteamAccount && SteamAccount.hasApiKey
                                         ? "API key stored securely" : "Steam Web API key"
                        color: Theme.foreground
                        placeholderTextColor: root.alpha(Theme.foreground, 0.42)
                        echoMode: TextInput.Password
                        font.family: Theme.fontFamily
                        background: Rectangle {
                            radius: Math.max(5, Theme.cornerRadius)
                            color: root.alpha(Theme.foreground, 0.045)
                            border.color: root.alpha(Theme.foreground, 0.15)
                        }
                    }
                    GlassButton {
                        compact: true
                        text: "SAVE KEY"
                        onClicked: {
                            SteamAccount.storeApiKey(apiKeyField.text)
                            apiKeyField.clear()
                        }
                    }
                    GlassButton {
                        compact: true
                        visible: SteamAccount ? SteamAccount.hasApiKey : false
                        text: "REMOVE"
                        onClicked: SteamAccount.removeApiKey()
                    }
                }
                GlassButton {
                    compact: true
                    text: "GET A KEY FROM STEAM"
                    onClicked: Qt.openUrlExternally("https://steamcommunity.com/dev/apikey")
                }
                Item { Layout.fillHeight: true }
                RowLayout {
                    spacing: 8
                    GlassButton {
                        compact: true
                        text: "CACHE -"
                        onClicked: Preferences.artworkCacheLimitMb -= 128
                    }
                    GlassButton {
                        compact: true
                        text: "CACHE +"
                        onClicked: Preferences.artworkCacheLimitMb += 128
                    }
                    GlassButton {
                        compact: true
                        text: "CLEAR ART"
                        onClicked: Achievements.clearCache()
                    }
                    Item { Layout.fillWidth: true }
                    GlassButton {
                        text: "CLOSE"
                        primary: true
                        onClicked: root.diagnosticsOpen = false
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        smokeReady = true
        libraryView.focusGrid()
    }

    Connections {
        target: SteamAccount
        enabled: SteamAccount !== null
        function onAchievementsUpdated(appId) {
            if (root.detailOpen && root.selectedGame.appId === appId) {
                root.selectedGame = Library.get(root.selectedIndex)
            }
        }
    }
}
