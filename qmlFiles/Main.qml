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
    property bool isSearchListExtended: false
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

        delegate: ChatBubble {
            id:chatBubble
            anchors.margins: 10 * 2
            width: Math.min(root.width, listView.width * 0.45)
            property string message: model.text
            property string time: model.time
            property string name: model.name
            property bool isOutgoing: model.isOutgoing
        }
    }

    ListModel {
        id: listModel
    }

    Rectangle {
        id: upLine
        color: "#17212b"
        height: 55
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
                    myProfileWindow.userProfile(nameText.text)
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

    MessageLine { id: downLine }

    function onNewMessage(name,message,time,isOutgoing) {
        //logger.qmlLog("INFO","Main.qml::onNewMessage","Upload new message to listModel");
        listModel.append({text: message, time: time, name: name, isOutgoing: isOutgoing});
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

    MouseArea{
        id:leaveSearchListArea
        anchors{
            left: centerLine.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        enabled: isSearchListExtended

        onClicked: {
            isSearchListExtended = !isSearchListExtended
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

    function onCheckActiveDialog(login,message,out,time)
    {
        logger.qmlLog("INFO","Main.qml::onCheckActiveDialog","Dialog active: " + (nameText.text === login));
        if (nameText.text === login)
        {
            if(out === "out") {
                onNewMessage(userlogin,message,time,true);
            } else {
                onNewMessage(login,message,time,false);
            }
        }
    }

    function onClearMainListView()
    {
        listModel.clear();
    }

    Component.onCompleted: {
        clearMainListView.connect(onClearMainListView);
        newMessage.connect(onNewMessage);
        checkActiveDialog.connect(onCheckActiveDialog);
        connectionError.connect(connectError);
        connectionSuccess.connect(connectSuccess);
    }
}
