import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Dialog {
    id: root
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: Math.min(600, rootWindow.width * 0.9)
    height: Math.min(550 + chatExample.height, rootWindow.height * 0.9)

    background: Rectangle {
        color: "#1e2a36"
        radius: 6
        border.color: "#626a72"
        border.width: 1/2
    }

    property color chatBackground: "#0e1621"
    property color incomingColor: "#182533"
    property color outgoingColor: "#2b5278"

    Flickable {
        id: flickable
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentWidth: container.width
        contentHeight: container.height

        Rectangle {
            id: container
            width: flickable.width
            height: 550 + chatExample.height
            color: "transparent"

            Rectangle {
                id: chatExample

                height: bubble1.height + bubble2.height + bubble3.height + 40
                color: chatBackground
                anchors{
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }

                ExampleChatBubble {
                    id: bubble1
                    isOutgoing: true
                    isRead: true
                    userName: "Bob"
                    message: "Hello"
                    time: "11:27"
                    anchors {
                        top: parent.top
                        topMargin: 10
                        left: parent.left
                        leftMargin: 10
                    }
                }

                ExampleChatBubble {
                    id: bubble2
                    isOutgoing: false
                    isRead: true
                    userName: "John"
                    message: "Hello, how are u?"
                    time: "11:29"
                    anchors {
                        top: bubble1.bottom
                        topMargin: 10
                        right: parent.right
                        rightMargin: 10
                    }
                }

                ExampleChatBubble {
                    id: bubble3
                    isOutgoing: true
                    isRead: false
                    userName: "Bob"
                    message: "I'm fine, and how are u?"
                    time: "11:29"
                    anchors {
                        top: bubble2.bottom
                        topMargin: 10
                        left: parent.left
                        leftMargin: 10
                    }
                }
            }

            Rectangle {
                id: chatBgButton
                height: 50
                color: "transparent"
                anchors {
                    top: chatExample.bottom
                    topMargin: 10
                    left: parent.left
                    right: parent.right
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: chatBgColorDialog.open()
                }

                Text {
                    id: chatLabel
                    text: "Chat background color"
                    color: "white"
                    font.pointSize: 14
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 20
                    }
                }
            }

            Rectangle {
                id: chatOutgCButton
                height: 50
                color: "transparent"
                anchors {
                    top: chatBgButton.bottom
                    topMargin: 10
                    left: parent.left
                    right: parent.right
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: chatOutgColorDialog.open()
                }

                Text {
                    id: outgCLabel
                    text: "Chat outgoing message color"
                    color: "white"
                    font.pointSize: 14
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 20
                    }
                }
            }

            Rectangle {
                id: chatIncCButton
                height: 50
                color: "transparent"
                anchors {
                    top: chatOutgCButton.bottom
                    topMargin: 10
                    left: parent.left
                    right: parent.right
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: chatIncColorDialog.open()
                }

                Text {
                    id: incCLabel
                    text: "Chat incoming message color"
                    color: "white"
                    font.pointSize: 14
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 20
                    }
                }
            }
        }
    }

    ColorDialog {
        id: chatBgColorDialog
        title: "Chat background color"
        selectedColor: chatBackground
        onAccepted: {
            chatBackground = selectedColor;
        }
    }

    ColorDialog {
        id: chatOutgColorDialog
        title: "Chat outgoing message color"
        selectedColor: outgoingColor
        onAccepted: {
            outgoingColor = selectedColor;
        }
    }

    ColorDialog {
        id: chatIncColorDialog
        title: "Chat incoming message color"
        selectedColor: incomingColor
        onAccepted: {
            incomingColor = selectedColor;
        }
    }

    onOpened: {
        overlay.opacity = 1
        root.opacity = 1
    }

    onClosed: {
        overlay.opacity = 0
        root.opacity = 0
    }
}
