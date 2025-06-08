import QtQuick
import QtQuick.Controls

Window {
    id: rootWindow
    width: 1000
    height: 500 + header.height
    visible: true
    title: qsTr("Messenger")
    flags: Qt.Window | Qt.FramelessWindowHint

    property bool maximized: false

    Rectangle {
        id: header
        height: 20
        anchors.left: parent.left
        anchors.right: parent.right
        color: "#19283b"
        MouseArea {
            id: headerDragArea
            anchors.fill: parent
            drag.axis: Drag.XAndYAxis

            onPressed: {
                if(!rootWindow.maximized){
                    rootWindow.startSystemMove()
                }
            }

            onDoubleClicked: {
                if (rootWindow.maximized) {
                    rootWindow.showNormal()
                    rootWindow.maximized = false
                } else {
                    rootWindow.showMaximized()
                    rootWindow.maximized = true
                }
            }
        }

        Button {
            id:maximizeWindowButton
            icon.source: rootWindow.maximized ? "../images/restoreW.svg" : "../images/maximize.svg"
            icon.cache: false
            icon.width: 20
            icon.height: 20
            icon.color: "white"
            anchors {
                verticalCenter: parent.verticalCenter
                right: hideAppButton.left
                rightMargin: 15
            }
            background: Rectangle { color: maximizeWindowButtonMouseArea.containsMouse ? "#2c4568" : "transparent"}
            width: 30
            height: 20
            MouseArea {
                id: maximizeWindowButtonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (rootWindow.maximized) {
                        rootWindow.showNormal()
                        rootWindow.maximized = false
                    } else {
                        rootWindow.showMaximized()
                        rootWindow.maximized = true
                    }
                }
            }
        }

        Button {
            id: hideAppButton
            icon.source: "../images/hide.svg"
            icon.cache: false
            icon.width: 15
            icon.height: 1
            icon.color: "white"
            anchors {
                verticalCenter: parent.verticalCenter
                right: closeAppButton.left
                rightMargin: 15
            }
            background: Rectangle { color: hideAppButtonMouseArea.containsMouse ? "#2c4568" : "transparent"}
            width: 30
            height: 20
            MouseArea {
                id: hideAppButtonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: rootWindow.showMinimized()
            }
        }

        Button {
            id: closeAppButton
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
            }
            contentItem: Text {
                text: "✕"
                font.pixelSize: 13
                color: "white"
                anchors {
                    top: parent.top
                    horizontalCenter: parent.horizontalCenter
                }
            }

            background: Rectangle { color: closeAppButtonMouseArea.containsMouse ? "#dc2f50" : "transparent"}
            width: 30
            height: 20
            MouseArea {
                id: closeAppButtonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: Qt.quit()
            }

        }
    }

    Loader {
        id: pageLoader
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        source: "LoginPage.qml"
    }

    MouseArea {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 5
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeVerCursor
        onPressed: { rootWindow.startSystemResize(Qt.TopEdge) }
    }

    MouseArea {
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: 5
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeVerCursor
        onPressed: { rootWindow.startSystemResize(Qt.BottomEdge) }
    }

    MouseArea {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: 5
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeHorCursor
        onPressed: { rootWindow.startSystemResize(Qt.LeftEdge) }
    }

    MouseArea {
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        width: 5
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeHorCursor
        onPressed: { rootWindow.startSystemResize(Qt.RightEdge) }
    }

    MouseArea {
        anchors {
            top: parent.top
            left: parent.left
        }
        width: 10
        height: 10
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeFDiagCursor
        onPressed: { rootWindow.startSystemResize(Qt.TopEdge | Qt.LeftEdge) }
    }

    MouseArea {
        anchors {
            top: parent.top
            right: parent.right
        }
        width: 10
        height: 10
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeBDiagCursor
        onPressed: { rootWindow.startSystemResize(Qt.TopEdge | Qt.RightEdge) }
    }

    MouseArea {
        anchors {
            bottom: parent.bottom
            left: parent.left
        }
        width: 10
        height: 10
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeBDiagCursor
        onPressed: { rootWindow.startSystemResize(Qt.BottomEdge | Qt.LeftEdge) }
    }

    MouseArea {
        anchors {
            bottom: parent.bottom
            right: parent.right
        }
        width: 10
        height: 10
        hoverEnabled: true
        enabled: !rootWindow.maximized
        cursorShape: rootWindow.maximized ? Qt.ArrowCursor : Qt.SizeFDiagCursor
        onPressed: { rootWindow.startSystemResize(Qt.BottomEdge | Qt.RightEdge) }
    }
}
