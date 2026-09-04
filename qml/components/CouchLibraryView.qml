import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

FocusScope {
    id: root

    required property var libraryModel
    property bool scanning: false
    property int currentIndex: 0
    property var currentGame: ({})
    property var pendingCurrent: null
    property bool searchOpen: false
    property string searchInitial: ""
    readonly property bool gridFocused: gameStrip.activeFocus
    readonly property real uiScale: Math.max(0.68, Math.min(1.25,
                                                           Math.min(width / 1920,
                                                                    height / 1080)))

    signal gameActivated(int index)
    signal favoriteToggled(int index)
    signal settingsRequested()
    signal desktopRequested()
    signal coverRequested(string source, string appId)

    Accessible.name: "Couch library"
    Accessible.role: Accessible.List

    function alpha(color, value) {
        return Qt.rgba(color.r, color.g, color.b, value)
    }

    function refreshCurrentGame() {
        if (libraryModel && currentIndex >= 0 && currentIndex < libraryModel.rowCount()) {
            currentGame = libraryModel.get(currentIndex)
        } else {
            currentGame = ({})
        }
    }

    function focusGrid() {
        if (gameStrip.count > 0) {
            gameStrip.forceActiveFocus(Qt.TabFocusReason)
        } else {
            settingsButton.forceActiveFocus(Qt.TabFocusReason)
        }
    }

    function toggleControls() {
        if (searchOpen) {
            return
        }
        if (gameStrip.activeFocus) {
            viewButton.forceActiveFocus(Qt.TabFocusReason)
        } else {
            focusGrid()
        }
    }

    function selectMode(mode) {
        libraryModel.mode = mode
        currentIndex = libraryModel.rowCount() > 0 ? 0 : -1
        Qt.callLater(focusGrid)
    }

    function openSearch() {
        searchInitial = libraryModel.searchText
        couchKeyboard.value = searchInitial
        searchOpen = true
        Qt.callLater(couchKeyboard.focusKeyboard)
    }

    function closeSearch(accepted) {
        if (!accepted) {
            libraryModel.searchText = searchInitial
        }
        searchOpen = false
        currentIndex = libraryModel.rowCount() > 0 ? 0 : -1
        Qt.callLater(function() {
            if (accepted) {
                root.focusGrid()
            } else {
                searchButton.forceActiveFocus(Qt.TabFocusReason)
            }
        })
    }

    onCurrentIndexChanged: refreshCurrentGame()
    onLibraryModelChanged: refreshCurrentGame()

    Connections {
        target: root.libraryModel
        function onModelAboutToBeReset() {
            if (root.currentIndex >= 0 && root.currentIndex < root.libraryModel.rowCount()) {
                const game = root.libraryModel.get(root.currentIndex)
                root.pendingCurrent = { source: game.source, runner: game.runner || "",
                                        appId: game.appId }
            } else {
                root.pendingCurrent = null
            }
        }
        function onModelReset() {
            const pending = root.pendingCurrent
            root.pendingCurrent = null
            const matched = pending
                            ? root.libraryModel.indexOf(pending.source, pending.runner,
                                                        pending.appId)
                            : -1
            root.currentIndex = matched >= 0 ? matched
                                : root.libraryModel.rowCount() > 0
                                  ? Math.max(0, Math.min(root.currentIndex,
                                                        root.libraryModel.rowCount() - 1))
                                  : -1
            root.refreshCurrentGame()
        }
        function onRowsInserted() { root.refreshCurrentGame() }
        function onRowsRemoved() { root.refreshCurrentGame() }
        function onDataChanged() { root.refreshCurrentGame() }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.darkerBackground
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0
                color: root.currentGame.accentStart
                       ? root.alpha(root.currentGame.accentStart, 0.38)
                       : root.alpha(Theme.accent, 0.24)
            }
            GradientStop {
                position: 0.52
                color: root.alpha(Theme.darkerBackground, 0.74)
            }
            GradientStop {
                position: 1
                color: Theme.darkerBackground
            }
        }
    }

    Image {
        id: heroArtwork
        anchors.fill: parent
        source: root.currentGame.heroPath || ""
        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectCrop
        sourceSize.width: Math.ceil(width * Math.max(1, Screen.devicePixelRatio) / 128) * 128
        sourceSize.height: Math.ceil(height * Math.max(1, Screen.devicePixelRatio) / 128) * 128
        opacity: status === Image.Ready ? 0.58 : 0

        Behavior on opacity {
            enabled: !Preferences.reducedMotion
            NumberAnimation { duration: 220 }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0; color: root.alpha(Theme.darkerBackground, 0.12) }
            GradientStop { position: 0.58; color: root.alpha(Theme.darkerBackground, 0.62) }
            GradientStop { position: 1; color: root.alpha(Theme.darkerBackground, 0.98) }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: root.alpha(Theme.darkerBackground, 0.94) }
            GradientStop { position: 0.58; color: root.alpha(Theme.darkerBackground, 0.34) }
            GradientStop { position: 1; color: root.alpha(Theme.darkerBackground, 0.12) }
        }
    }

    RowLayout {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 32 * root.uiScale
        anchors.leftMargin: 54 * root.uiScale
        anchors.rightMargin: 54 * root.uiScale
        spacing: 12 * root.uiScale

        Row {
            spacing: 12 * root.uiScale
            Layout.alignment: Qt.AlignVCenter

            Image {
                width: 42 * root.uiScale
                height: width
                source: "qrc:/icons/resources/icons/io.github.tsouth89.Omakade.svg"
                sourceSize: Qt.size(96, 96)
                Accessible.ignored: true
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0

                Text {
                    text: "OMAKADE"
                    color: Theme.brightForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: 18 * root.uiScale
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                }
                Text {
                    text: "COUCH MODE"
                    color: Theme.accent
                    font.family: Theme.fontFamily
                    font.pixelSize: 9 * root.uiScale
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.4
                }
            }
        }

        Item { Layout.fillWidth: true }

        Row {
            spacing: 7 * root.uiScale
            Layout.alignment: Qt.AlignVCenter

            GlassButton {
                id: allButton
                text: "ALL"
                compact: true
                displayScale: Math.max(1, root.uiScale * 1.18)
                selected: root.libraryModel.mode === 0
                onClicked: root.selectMode(0)
                KeyNavigation.right: favoritesButton
                KeyNavigation.down: viewButton
            }
            GlassButton {
                id: favoritesButton
                text: "FAVORITES"
                compact: true
                displayScale: Math.max(1, root.uiScale * 1.18)
                selected: root.libraryModel.mode === 1
                onClicked: root.selectMode(1)
                KeyNavigation.left: allButton
                KeyNavigation.right: recentButton
                KeyNavigation.down: viewButton
            }
            GlassButton {
                id: recentButton
                text: "RECENT"
                compact: true
                displayScale: Math.max(1, root.uiScale * 1.18)
                selected: root.libraryModel.mode === 2
                onClicked: root.selectMode(2)
                KeyNavigation.left: favoritesButton
                KeyNavigation.right: searchButton
                KeyNavigation.down: favoriteButton
            }
            GlassButton {
                id: searchButton
                objectName: "couchSearchButton"
                text: root.libraryModel.searchText.length > 0
                      ? "SEARCH · "
                        + root.libraryModel.searchText.substring(0, 12).toUpperCase()
                        + (root.libraryModel.searchText.length > 12 ? "…" : "")
                      : "SEARCH"
                compact: true
                displayScale: Math.max(1, root.uiScale * 1.18)
                onClicked: root.openSearch()
                KeyNavigation.left: recentButton
                KeyNavigation.right: settingsButton
                KeyNavigation.down: favoriteButton
            }
            GlassButton {
                id: settingsButton
                objectName: "couchSettingsButton"
                text: "SETTINGS"
                compact: true
                displayScale: Math.max(1, root.uiScale * 1.18)
                onClicked: root.settingsRequested()
                KeyNavigation.left: searchButton
                KeyNavigation.right: desktopButton
                KeyNavigation.down: favoriteButton
            }
            GlassButton {
                id: desktopButton
                objectName: "couchDesktopButton"
                text: "DESKTOP"
                compact: true
                displayScale: Math.max(1, root.uiScale * 1.18)
                onClicked: root.desktopRequested()
                KeyNavigation.left: settingsButton
                KeyNavigation.down: favoriteButton
            }
        }
    }

    Item {
        id: heroCoverFrame
        anchors.top: topBar.bottom
        anchors.topMargin: 44 * root.uiScale
        anchors.right: parent.right
        anchors.rightMargin: 116 * root.uiScale
        width: 330 * root.uiScale
        height: width * 1.5
        visible: gameStrip.count > 0 && root.width >= 1200

        Rectangle {
            anchors.fill: parent
            anchors.margins: 14 * root.uiScale
            rotation: 7
            radius: 18 * root.uiScale
            color: root.alpha(root.currentGame.accentEnd || Theme.accent, 0.18)
            border.color: root.alpha(Theme.brightForeground, 0.10)
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 8 * root.uiScale
            rotation: -4
            radius: 18 * root.uiScale
            color: root.alpha(root.currentGame.accentStart || Theme.green, 0.24)
            border.color: root.alpha(Theme.brightForeground, 0.12)
        }

        Rectangle {
            id: featuredCover
            anchors.fill: parent
            radius: 16 * root.uiScale
            clip: true
            border.width: 2
            border.color: root.alpha(Theme.brightForeground, 0.22)
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: root.currentGame.accentStart || Theme.accent
                }
                GradientStop {
                    position: 1
                    color: root.currentGame.accentEnd || Theme.darkerBackground
                }
            }

            Image {
                anchors.fill: parent
                source: root.currentGame.coverPath || ""
                asynchronous: true
                cache: false
                fillMode: Image.PreserveAspectCrop
                sourceSize.width: Math.ceil(width * Math.max(1, Screen.devicePixelRatio) / 64) * 64
                sourceSize.height: Math.ceil(height * Math.max(1, Screen.devicePixelRatio) / 64) * 64
            }

            Rectangle {
                anchors.fill: parent
                visible: !root.currentGame.coverPath
                color: "transparent"

                Rectangle {
                    width: parent.width * 0.9
                    height: width
                    radius: width / 2
                    x: parent.width * 0.44
                    y: -height * 0.2
                    color: root.alpha(Theme.brightForeground, 0.11)
                }
                Rectangle {
                    width: parent.width * 0.72
                    height: width
                    radius: width / 2
                    x: -width * 0.38
                    y: parent.height * 0.5
                    color: root.alpha(Theme.darkerBackground, 0.2)
                }
                Text {
                    anchors.centerIn: parent
                    text: root.currentGame.coverMark || "◇"
                    color: root.alpha(Theme.brightForeground, 0.88)
                    font.family: Theme.fontFamily
                    font.pixelSize: 92 * root.uiScale
                    font.weight: Font.Light
                }
            }
        }
    }

    Column {
        id: heroCopy
        anchors.left: parent.left
        anchors.leftMargin: 64 * root.uiScale
        anchors.bottom: gameStrip.top
        anchors.bottomMargin: 42 * root.uiScale
        width: Math.min(parent.width * 0.58, 920 * root.uiScale)
        spacing: 12 * root.uiScale
        visible: gameStrip.count > 0

        Text {
            width: parent.width
            text: ((root.currentGame.source || "LIBRARY")
                   + (root.currentGame.year ? "  ·  " + root.currentGame.year : "")).toUpperCase()
            textFormat: Text.PlainText
            color: Theme.accent
            font.family: Theme.fontFamily
            font.pixelSize: 13 * root.uiScale
            font.weight: Font.Bold
            font.letterSpacing: 1.8
            elide: Text.ElideRight
        }

        Image {
            id: logoArtwork
            width: Math.min(parent.width * 0.72, 560 * root.uiScale)
            height: 120 * root.uiScale
            source: root.currentGame.logoPath || ""
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignLeft
            visible: status === Image.Ready
            sourceSize.width: Math.ceil(width * Math.max(1, Screen.devicePixelRatio))
        }

        Text {
            width: parent.width
            text: root.currentGame.title || ""
            textFormat: Text.PlainText
            color: Theme.brightForeground
            font.family: Theme.fontFamily
            font.pixelSize: 50 * root.uiScale
            font.weight: Font.Bold
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            visible: !logoArtwork.visible
        }

        Text {
            width: parent.width
            text: root.currentGame.description || root.currentGame.subtitle || ""
            textFormat: Text.PlainText
            color: root.alpha(Theme.brightForeground, 0.78)
            font.family: Theme.fontFamily
            font.pixelSize: 17 * root.uiScale
            lineHeight: 1.22
            wrapMode: Text.Wrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }

        Row {
            spacing: 10 * root.uiScale

            GlassButton {
                id: viewButton
                objectName: "couchViewButton"
                text: "VIEW GAME"
                iconText: "▶"
                primary: true
                displayScale: Math.max(1, root.uiScale * 1.2)
                enabled: root.currentIndex >= 0
                onClicked: root.gameActivated(root.currentIndex)
                KeyNavigation.up: allButton
                KeyNavigation.right: favoriteButton
                KeyNavigation.down: gameStrip
            }
            GlassButton {
                id: favoriteButton
                objectName: "couchFavoriteButton"
                text: root.currentGame.favorite ? "FAVORITED" : "FAVORITE"
                iconText: root.currentGame.favorite ? "♥" : "♡"
                displayScale: Math.max(1, root.uiScale * 1.2)
                enabled: root.currentIndex >= 0
                onClicked: root.favoriteToggled(root.currentIndex)
                KeyNavigation.up: settingsButton
                KeyNavigation.left: viewButton
                KeyNavigation.down: gameStrip
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 14 * root.uiScale
        visible: gameStrip.count === 0

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.scanning ? "SCANNING YOUR LIBRARY" : "NO GAMES HERE"
            color: Theme.brightForeground
            font.family: Theme.fontFamily
            font.pixelSize: 26 * root.uiScale
            font.weight: Font.Bold
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.scanning ? "Looking for installed games and artwork."
                                : "Change the library view or rescan from Settings."
            color: Theme.mutedText
            font.family: Theme.fontFamily
            font.pixelSize: 15 * root.uiScale
        }
    }

    ListView {
        id: gameStrip
        objectName: "couchGameStrip"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: hintBar.top
        anchors.leftMargin: 52 * root.uiScale
        anchors.rightMargin: 52 * root.uiScale
        anchors.bottomMargin: 22 * root.uiScale
        height: 260 * root.uiScale
        orientation: ListView.Horizontal
        spacing: 18 * root.uiScale
        clip: true
        model: root.libraryModel
        currentIndex: root.currentIndex
        keyNavigationEnabled: true
        highlightMoveDuration: Preferences.reducedMotion ? 0 : 130
        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: width * 0.08
        preferredHighlightEnd: width * 0.72
        boundsBehavior: Flickable.StopAtBounds

        onCurrentIndexChanged: root.currentIndex = currentIndex

        Keys.onUpPressed: function(event) {
            viewButton.forceActiveFocus(Qt.TabFocusReason)
            event.accepted = true
        }
        Keys.onReturnPressed: function(event) {
            if (currentIndex >= 0) {
                root.gameActivated(currentIndex)
            }
            event.accepted = true
        }
        Keys.onEnterPressed: function(event) {
            if (currentIndex >= 0) {
                root.gameActivated(currentIndex)
            }
            event.accepted = true
        }

        delegate: FocusScope {
            id: card
            required property int index
            required property string title
            required property string subtitle
            required property string coverPath
            required property string coverMark
            required property string source
            required property string appId
            required property bool favorite
            required property color accentStart
            required property color accentEnd

            width: 146 * root.uiScale
            height: gameStrip.height
            Accessible.name: title
            Accessible.role: Accessible.ListItem

            Component.onCompleted: {
                if (coverPath.length === 0) {
                    root.coverRequested(source, appId)
                }
            }

            Rectangle {
                id: cover
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: width * 1.5
                radius: 10 * root.uiScale
                clip: true
                border.width: gameStrip.currentIndex === card.index ? 4 : 1
                border.color: gameStrip.currentIndex === card.index
                              ? Theme.accent : root.alpha(Theme.foreground, 0.18)
                gradient: Gradient {
                    GradientStop { position: 0; color: card.accentStart }
                    GradientStop { position: 1; color: card.accentEnd }
                }

                Image {
                    anchors.fill: parent
                    source: card.coverPath
                    asynchronous: true
                    cache: false
                    fillMode: Image.PreserveAspectCrop
                    sourceSize.width: Math.ceil(width * Math.max(1, Screen.devicePixelRatio) / 64) * 64
                    sourceSize.height: Math.ceil(height * Math.max(1, Screen.devicePixelRatio) / 64) * 64
                }

                Text {
                    anchors.centerIn: parent
                    visible: card.coverPath.length === 0
                    text: card.coverMark
                    color: root.alpha(Theme.brightForeground, 0.86)
                    font.family: Theme.fontFamily
                    font.pixelSize: 42 * root.uiScale
                }

                Rectangle {
                    visible: card.favorite
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 8 * root.uiScale
                    width: 28 * root.uiScale
                    height: width
                    radius: width / 2
                    color: root.alpha(Theme.darkerBackground, 0.72)

                    Text {
                        anchors.centerIn: parent
                        text: "♥"
                        color: Theme.brightForeground
                        font.pixelSize: 12 * root.uiScale
                    }
                }
            }

            Text {
                anchors.top: cover.bottom
                anchors.topMargin: 9 * root.uiScale
                width: parent.width
                text: card.title
                textFormat: Text.PlainText
                color: gameStrip.currentIndex === card.index
                       ? Theme.brightForeground : Theme.foreground
                font.family: Theme.fontFamily
                font.pixelSize: 14 * root.uiScale
                font.weight: gameStrip.currentIndex === card.index ? Font.Bold : Font.Medium
                elide: Text.ElideRight
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    gameStrip.currentIndex = card.index
                    gameStrip.forceActiveFocus(Qt.MouseFocusReason)
                }
                onDoubleClicked: root.gameActivated(card.index)
            }

            opacity: gameStrip.currentIndex === card.index ? 1 : 0.78
            Behavior on opacity {
                enabled: !Preferences.reducedMotion
                NumberAnimation { duration: 130; easing.type: Easing.OutCubic }
            }
        }
    }

    Row {
        id: hintBar
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 54 * root.uiScale
        anchors.bottomMargin: 22 * root.uiScale
        spacing: 22 * root.uiScale

        Repeater {
            model: [
                { glyph: Controller.primaryGlyph, label: "OPEN" },
                { glyph: Controller.favoriteGlyph, label: "FAVORITE" },
                { glyph: Controller.toolbarGlyph, label: "CONTROLS" },
                { glyph: "START", label: "DESKTOP" }
            ]

            Row {
                required property var modelData
                spacing: 7 * root.uiScale

                Rectangle {
                    width: Math.max(31 * root.uiScale, glyphText.implicitWidth + 14 * root.uiScale)
                    height: 31 * root.uiScale
                    radius: height / 2
                    color: root.alpha(Theme.foreground, 0.12)
                    border.color: root.alpha(Theme.foreground, 0.22)

                    Text {
                        id: glyphText
                        anchors.centerIn: parent
                        text: modelData.glyph
                        color: Theme.brightForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: 11 * root.uiScale
                        font.weight: Font.Bold
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.label
                    color: Theme.mutedText
                    font.family: Theme.fontFamily
                    font.pixelSize: 12 * root.uiScale
                    font.weight: Font.DemiBold
                    font.letterSpacing: 0.8
                }
            }
        }
    }

    CouchKeyboard {
        id: couchKeyboard
        objectName: "couchKeyboard"
        anchors.fill: parent
        visible: root.searchOpen
        enabled: visible
        z: 50

        onValueEdited: function(value) {
            root.libraryModel.searchText = value
            root.currentIndex = root.libraryModel.rowCount() > 0 ? 0 : -1
        }
        onAccepted: function(value) {
            root.libraryModel.searchText = value
            root.closeSearch(true)
        }
        onCanceled: root.closeSearch(false)
    }

    Component.onCompleted: {
        currentIndex = libraryModel.rowCount() > 0 ? 0 : -1
        refreshCurrentGame()
        Qt.callLater(focusGrid)
    }
}
