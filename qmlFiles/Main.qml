import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Window {
    id: root
    width: 1000
    height: 500
    visible: true
    color: "#0e1621"
    title: qsTr("Regagram")

    property bool isProfileExtended: false

    Rectangle {
        id: leftLine
        color: "#0e1621"
        border.color: "black"
        border.width: 0.5
        height: root.height
        width: 54
        anchors.left:  parent.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top

        Rectangle {
            id: profile
            color: "#0e1621"
            border.color: "green"
            border.width: 0.5
            height: 54
            anchors.left:  parent.left
            anchors.right: parent.right
            anchors.top: parent.top

            MouseArea {
                id: profileMouseArea
                anchors.fill: parent
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                enabled: !isProfileExtended

                onClicked: {
                    isProfileExtended = !isProfileExtended
                }

                onPressed: {
                    profile.color = "gray"
                }

                onReleased: {
                    profile.color = "#0e1621"
                }
            }
        }
    }

    Rectangle {
        id: centerLine
        color: "#17212b"
        border.color: "black"
        border.width: 0.5
        height: root.height
        width: root.width - (root.width / 2 + root.width / 4) - 54
        anchors.left:  parent.left
        anchors.leftMargin: 54
        anchors.bottom: parent.bottom
        anchors.top: parent.top
    }

    ListView {
        id: listView
        anchors.topMargin: 5
        anchors.top : upLine.bottom
        anchors.left: centerLine.right
        anchors.leftMargin: 5
        anchors.right: parent.right
        anchors.bottom: downLine.top
        spacing: 5
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            background: Rectangle {
                implicitWidth: 10
                color: root.color
            }
            contentItem: Rectangle {
                implicitWidth: 10
                color: "gray"
                radius: 5
            }
        }

        model: listModel

        delegate: Loader {
            anchors.margins: 10 * 2
            width: Math.min(root.width, listView.width * 0.45)
            active: true
            sourceComponent: model.isOutgoing ? outgoingDelegate : incomingDelegate
            property string textload: model.text
            property string timeload: model.time
            property string nameload: model.name
        }

        Component {
            id: outgoingDelegate
            ChatBubbleOut {
                anchors.right: parent.right
                text: textload
                time: timeload
                name: nameload
            }
        }

        Component {
            id: incomingDelegate
            ChatBubbleIn {
                anchors.left: parent.left
                text: textload
                time: timeload
                name: nameload
            }
        }
    }

    ListModel {
        id: listModel
        ListElement {
            text: "Wassaaa, are you here?"
            time: "no:time"
            name: "xdlmk"
            isOutgoing: false
        }
    }

    Rectangle {
        id: upLine
        color: "#17212b"
        border.color: "black"
        border.width: 0.5
        height: 60
        anchors.left:  centerLine.right
        anchors.top: parent.top
        anchors.right: parent.right

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            id: nameText
            text: "Chat"
            font.pointSize: 10
            color: "white"
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            id: valueText
            text: "5 participants"
            font.pointSize: 8
            color: "grey"
        }
    }

    MessageLine {
        id: downLine
        border.color: "black"
        border.width: 0.5
        onNewMessage: {
            var newMsg = {};
            newMsg.text = msg;
            newMsg.time = Qt.formatTime(new Date(), "hh:mm");
            newMsg.name = userlogin;
            newMsg.isOutgoing = true;
            client.sendToServer(msg, newMsg.name);
        }
    }

    function onInMessage(name) {
        var newMsg = {};
        newMsg.text = messageFrom;
        newMsg.time = Qt.formatTime(new Date(), "hh:mm");
        newMsg.name = name;
        newMsg.isOutgoing = false;
        listModel.append(newMsg);
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }

    function onOutMessage(name) {
        var newMsg = {};
        newMsg.text = messageFrom;
        newMsg.time = Qt.formatTime(new Date(), "hh:mm");
        newMsg.name = name;
        newMsg.isOutgoing = true;
        listModel.append(newMsg);
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }

    Rectangle {
        id: connectRect
        visible: false
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        color: "#9945464f"
        height: 25

        Text {
            id: textConnect
            anchors.centerIn: parent
            color: "#99FFFFFF"
            font.pointSize: 10
            text: "Connection unsuccessful, try connecting again"
        }
    }

    Rectangle {
        id: profileWindow
        width: 250
        height: root.height
        color: "#1e2a36"
        border.color: "black"
        border.width: 0.5
        x: isProfileExtended ? 0 : -width  // Используем свойство x для управления позицией окна
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        Behavior on x {
            NumberAnimation {
                duration: 500
                easing.type: Easing.InOutQuad
            }
        }

        Column {
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 2
            spacing: 10

            Rectangle {
                id: profileImageContainer
                width: 50
                height: 50
                radius: 25
                border.color: "green"
                border.width: 0.5
                color: "transparent"
                anchors.left: parent.left
                anchors.leftMargin: 10

                Image {
                    id: profileImage
                    source: "../images/logo.png"
                    width: parent.width
                    height: parent.height
                    anchors.left: parent.left
                    anchors.top: parent.top
                    fillMode: Image.PreserveAspectCrop
                    clip: true
                }
            }

            Text {
                id: userLoginText
                text: userlogin
                color: "white"
                font.pointSize: 10
                font.bold: true
                anchors.left: parent.left
                anchors.leftMargin: 10
                visible: true
            }

            Rectangle {
                id: buttonLeave
                color: "#1e2a36"
                height: 40
                anchors.left:  parent.left
                anchors.right: parent.right

                Text {
                    id: leaveText
                    text: "Выйти"
                    color: "#cc353e"
                    font.pointSize: 10
                    font.bold: true
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    anchors.leftMargin: 10
                    visible: true
                }

                MouseArea {
                    id: buttonLeaveMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        console.log("Mouse clicked");
                        client.logout();
                    }

                    onEntered: {
                        buttonLeave.color = "#626a72";
                    }

                    onExited: {
                        buttonLeave.color = "#1e2a36";
                    }
                }
            }
        }
    }

    MouseArea{
        id:leaveProfileArea
        anchors.left: profileWindow.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        enabled: isProfileExtended

        onClicked: {
            isProfileExtended = !isProfileExtended
        }
    }

    function connectError() {
        connectRect.visible = true;
    }

    function connectSuccess() {
        connectRect.visible = false;
    }

    Component.onCompleted: {
        newInMessage.connect(onInMessage);
        newOutMessage.connect(onOutMessage);
        errorWithConnect.connect(connectError);
        connectionSuccess.connect(connectSuccess)
    }
}
