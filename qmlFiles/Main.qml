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
    property string avatarSource: "../images/avatar.png"

    Rectangle {
        id: leftLine
        color: "#0e1621"
        height: root.height
        width: 54
        anchors.left:  parent.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top

        Rectangle {
            id: profile
            color: "#0e1621"
            height: 54
            anchors.left:  parent.left
            anchors.right: parent.right
            anchors.top: parent.top

            Rectangle {
                id: colorOverlayProfile
                anchors.fill: parent
                anchors.margins: 1
                color: "#262d37"
                opacity: 0

                visible: false

                Behavior on opacity {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.OutQuad
                    }
                }
            }
            Image {
                id: listImage
                source: "../images/profile.png"
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: profileMouseArea
                anchors.fill: parent
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                enabled: !isProfileExtended

                onClicked: {
                    isProfileExtended = !isProfileExtended
                }

                onPressed: {
                    colorOverlayProfile.visible = true
                    colorOverlayProfile.opacity = 1
                }

                onReleased: {
                    colorOverlayProfile.opacity = 0
                }
            }
        }
    }

    ChatsList {
        id: centerLine
        height: root.height
        width: root.width - (root.width / 2 + root.width / 4) - 54

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
            visible: upLine.currentState === "default" ? false : true
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
            text: "FFFFFflsq12"
            time: "no:time"
            name: "xdlmk"
            isOutgoing: false
        }
    }

    Rectangle {
        id: upLine
        color: "#17212b"
        height: 60
        anchors.left:  centerLine.right
        anchors.top: parent.top
        anchors.right: parent.right
        property string currentState: "default"
        property int user_id: 0
        visible: currentState === "default" ? false : true

        Text {
            id: nameText
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            text: "Chat"
            font.pointSize: 10
            color: "white"
            MouseArea {
                id: openProfileMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    overlay.visible = true
                    myProfileWindow.open()
                }
            }
        }

        Text {
            id: valueText
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            text: "5 participants"
            font.pointSize: 8
            color: "grey"
        }
    }

    MessageLine {
        id: downLine
    }

    function onInMessage(name,message,time) {
        var newMsg = {};
        newMsg.text = message;
        newMsg.time = time;
        newMsg.name = name;
        newMsg.isOutgoing = false;
        listModel.append(newMsg);
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }

    function onOutMessage(name,message,time) {
        console.log("onOutMessage");
        var newMsg = {};
        newMsg.text = message;
        newMsg.time = time;
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

    ProfilePanel{
        id:profileWindow
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        x: isProfileExtended ? 0 : -width
        Behavior on x {
            NumberAnimation {
                duration: 500
                easing.type: Easing.InOutQuad
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
    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "#80000000"
        visible: false
        opacity: 0
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    MyProfile{
        id: myProfileWindow
    }

    function connectError() {
        connectRect.visible = true;
    }

    function connectSuccess() {
        connectRect.visible = false;
    }

    function onCheckActiveDialog(userlogin)
    {
        console.log("onCheckActiveDialog: " + nameText.text + "<-ActiveDialog " + userlogin + "<-checkDialog ");
        if (nameText.text === userlogin)
        {
            loadingPersonalChat(userlogin);
        }
    }

    function onClearMainListView()
    {
        listModel.clear();
    }

    Component.onCompleted: {
        clearMainListView.connect(onClearMainListView);
        newInMessage.connect(onInMessage);
        newOutMessage.connect(onOutMessage);
        checkActiveDialog.connect(onCheckActiveDialog);
        connectionError.connect(connectError);
        connectionSuccess.connect(connectSuccess);
    }
}
