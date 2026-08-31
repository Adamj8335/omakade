import QtQuick
import QtQuick.Controls

Item {
    id: root

    Accessible.name: "Game library"
    Accessible.role: Accessible.List

    required property var libraryModel
    property alias currentIndex: grid.currentIndex
    readonly property int count: grid.count
    property bool scanning: false
    property string emptyTitle: "No games found"
    property string emptyMessage: "Try a different search or library view."

    signal gameActivated(int index)
    signal favoriteToggled(int index)
    signal refreshRequested()

    function focusGrid() {
        if (grid.count > 0) {
            grid.forceActiveFocus()
        }
    }

    GridView {
        id: grid
        anchors.fill: parent
        clip: true
        model: root.libraryModel
        boundsBehavior: Flickable.StopAtBounds
        keyNavigationEnabled: true
        highlightFollowsCurrentItem: true
        highlightMoveDuration: 110
        cacheBuffer: height
        reuseItems: true
        focus: true
        currentIndex: count > 0 ? Math.min(currentIndex, count - 1) : -1

        Keys.onReturnPressed: function(event) {
            if (currentIndex >= 0) {
                root.gameActivated(currentIndex)
                event.accepted = true
            }
        }
        Keys.onEnterPressed: function(event) {
            if (currentIndex >= 0) {
                root.gameActivated(currentIndex)
                event.accepted = true
            }
        }
        Keys.onSpacePressed: function(event) {
            if (currentIndex >= 0) {
                root.gameActivated(currentIndex)
                event.accepted = true
            }
        }
        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_F && currentIndex >= 0) {
                root.favoriteToggled(currentIndex)
                event.accepted = true
            }
        }

        readonly property int columns: Math.max(2, Math.floor(width / 188))
        cellWidth: width / columns
        cellHeight: Math.round(cellWidth * 1.42) + 64

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 5
            contentItem: Rectangle {
                radius: 3
                color: Qt.rgba(Theme.foreground.r, Theme.foreground.g, Theme.foreground.b, 0.22)
            }
        }

        delegate: Item {
            id: delegateRoot
            required property int index
            required property string title
            required property string subtitle
            required property int hours
            required property int progress
            required property bool favorite
            required property color accentStart
            required property color accentEnd
            required property string coverMark
            required property string coverPath

            width: grid.cellWidth
            height: grid.cellHeight

            GameCard {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.topMargin: 7
                anchors.bottomMargin: 7
                title: delegateRoot.title
                subtitle: delegateRoot.subtitle
                hours: delegateRoot.hours
                progress: delegateRoot.progress
                favorite: delegateRoot.favorite
                accentStart: delegateRoot.accentStart
                accentEnd: delegateRoot.accentEnd
                coverMark: delegateRoot.coverMark
                coverPath: delegateRoot.coverPath
                current: grid.currentIndex === delegateRoot.index
                focus: current

                onActiveFocusChanged: {
                    if (activeFocus) {
                        grid.currentIndex = delegateRoot.index
                    }
                }
                onActivated: root.gameActivated(delegateRoot.index)
                onFavoriteToggled: root.favoriteToggled(delegateRoot.index)
            }
        }

        onCountChanged: {
            if (count > 0 && currentIndex < 0) {
                currentIndex = 0
            }
        }
    }

    Column {
        visible: grid.count === 0
        anchors.centerIn: parent
        spacing: 12

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "◇"
            color: Theme.accent
            font.family: Theme.fontFamily
            font.pixelSize: 42
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.scanning ? "Scanning Steam" : root.emptyTitle
            color: Theme.foreground
            font.family: Theme.fontFamily
            font.pixelSize: 16
            font.weight: Font.DemiBold
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.scanning ? "Looking for installed games and local artwork." : root.emptyMessage
            color: Theme.mutedText
            font.family: Theme.fontFamily
            font.pixelSize: 11
        }
        GlassButton {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: !root.scanning
            text: "RESCAN"
            compact: true
            onClicked: root.refreshRequested()
        }
    }
}
