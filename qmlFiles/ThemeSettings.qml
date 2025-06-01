import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Dialog {
    id: root
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: Math.min(600, rootWindow.width * 0.9)
    height: Math.min(chatExample.height + themeSettingsText.implicitHeight + chatBgButton.implicitHeight + chatOutgCButton.implicitHeight + chatIncCButton.implicitHeight + confirmButton.implicitHeight + 140, rootWindow.height * 0.9)

    background: Rectangle {
        color: Qt.lighter(themeManager.chatBackground)
        radius: 6
    }

    property color chatBackground: themeManager.chatBackground
    property color incomingColor: themeManager.incomingColor
    property color outgoingColor: themeManager.outgoingColor

    Flickable {
        id: flickable
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentWidth: container.width
        contentHeight: container.height + 140

        Rectangle {
            id: container
            width: flickable.width
            height: chatExample.implicitHeight + themeSettingsText.implicitHeight + chatBgButton.implicitHeight + chatOutgCButton.implicitHeight + chatIncCButton.implicitHeight + confirmButton.implicitHeight
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

            Text {
                id:themeSettingsText
                anchors {
                    top: chatExample.bottom
                    topMargin: 10
                    left: parent.left
                    rightMargin: 10
                }
                text: "Theme color settings"
                font.pointSize: 11
                font.bold: true
                color: outgoingColor
            }

            Rectangle {
                id: chatBgButton
                height: 30
                color: chatBgButtonMouseArea.containsMouse ? Qt.lighter(chatBackground, 1.75) : Qt.lighter(chatBackground)
                anchors {
                    top: themeSettingsText.bottom
                    topMargin: 10
                    left: parent.left
                    right: parent.right
                }
                MouseArea {
                    id: chatBgButtonMouseArea
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    anchors.fill: parent
                    onClicked: chatBgColorDialog.open()
                }

                Text {
                    id: chatLabel
                    text: "Chat background color"
                    color: "white"
                    font.pointSize: 11
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 20
                    }
                }
            }

            Rectangle {
                id: chatOutgCButton
                height: 30
                color: chatOutgCButtonMouseArea.containsMouse ? Qt.lighter(chatBackground, 1.75) : Qt.lighter(chatBackground)
                anchors {
                    top: chatBgButton.bottom
                    left: parent.left
                    right: parent.right
                }
                MouseArea {
                    id: chatOutgCButtonMouseArea
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    anchors.fill: parent
                    onClicked: chatOutgColorDialog.open()
                }

                Text {
                    id: outgCLabel
                    text: "Chat outgoing message color"
                    color: "white"
                    font.pointSize: 11
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 20
                    }
                }
            }

            Rectangle {
                id: chatIncCButton
                height: 30
                color: chatIncCButtonMouseArea.containsMouse ? Qt.lighter(chatBackground, 1.75) : Qt.lighter(chatBackground)
                anchors {
                    top: chatOutgCButton.bottom
                    left: parent.left
                    right: parent.right
                }
                MouseArea {
                    id: chatIncCButtonMouseArea
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    anchors.fill: parent
                    onClicked: chatIncColorDialog.open()
                }

                Text {
                    id: incCLabel
                    text: "Chat incoming message color"
                    color: "white"
                    font.pointSize: 11
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 20
                    }
                }
            }

            Text {
                id:confirmButton
                anchors {
                    top: chatIncCButton.bottom
                    bottomMargin: 10
                    right: parent.right
                    rightMargin: 10
                }

                text: "Confirm"
                font.pointSize: 11
                color: outgoingColor
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        themeManager.chatBackground = chatBackground;
                        themeManager.incomingColor = incomingColor;
                        themeManager.outgoingColor = outgoingColor;
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
