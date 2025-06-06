import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Dialog {
    id: myProfileDialogRoot
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0

    background: Rectangle {
        color: adjustColor(themeManager.chatBackground, 1.5, false)
        radius: 6
    }
    width: 400
    height: 500

    property string login: userlogin
    property int user_id: user_id

    Text{
        id:myProfileText
        text: "My profile"
        color: isColorLight(myProfileDialogRoot.background.color) ? "black" : "white"
        font.pointSize: 15
        font.bold: true

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    Text{
        id:editButton
        text: "\u270E"
        color: isColorLight(myProfileDialogRoot.background.color) ? "black" : "white"
        font.pointSize: 15
        font.bold: false

        anchors.right: closeButton.left
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        visible: true
        MouseArea {
            id: editButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                myProfileEdit.open()
            }
        }
    }

    Text{
        id:closeButton
        text: "✕"
        color: isColorLight(myProfileDialogRoot.background.color) ? "black" : "white"
        font.pointSize: 15
        font.bold: true

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        MouseArea {
            id: closeButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                myProfileWindow.close()
            }
        }
    }

    SmartImage {
        id:userAvatar
        width: 80
        height: 80
        anchors {
            top: myProfileText.bottom
            topMargin: 30
            left: myProfileText.left
        }
        textImage: userLoginText.text
        source: user_id !== 0 ? avatarSource + user_id + ".png?" + timestamp : ""
        visible: source !== ""
        fillMode: Image.PreserveAspectFit
    }

    Text{
        id:userLoginText
        text: login
        font.pointSize: 12
        color: isColorLight(myProfileDialogRoot.background.color) ? "black" : "white"
        font.bold: true
        anchors.left: userAvatar.right
        anchors.leftMargin: 12
        anchors.top: userAvatar.top
        anchors.topMargin: 12
    }

    Rectangle{
        id:defLine
        height: 6
        width:parent.width + 22
        color: adjustColor(themeManager.chatBackground, 2.25, false)
        anchors.left: parent.left
        anchors.leftMargin: -11
        anchors.top: userAvatar.bottom
        anchors.topMargin: 24
    }

    Text{
        id:informationIcon
        text: "ⓘ"
        font.pointSize: 18
        color: isColorLight(myProfileDialogRoot.background.color) ? "black" : "white"
        font.bold: true
        anchors.left: userAvatar.left
        anchors.top: defLine.bottom
        anchors.topMargin: 12
    }


    Item{
        anchors.left: informationIcon.right
        anchors.leftMargin: 36
        anchors.top: defLine.bottom
        anchors.topMargin: 12
        Text{
            id:userCheckLoginText
            text: userCheckLoginMouseArea.containsMouse ? "<u>@" + login + "</u>" : "@" + login
            font.pointSize: 12
            color: "#0078D4"
            font.bold: false
            anchors.top: parent.top
            textFormat: Text.RichText
            MouseArea {
                id: userCheckLoginMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    //copy
                }
            }
        }
        Text{
            id:underUserCheckLoginText
            text: "Username"
            font.pointSize: 10
            color: isColorLight(myProfileDialogRoot.background.color) ? "black" : "white"
            anchors.top: userCheckLoginText.bottom
            anchors.topMargin: 3
            anchors.left: userCheckLoginText.left
            anchors.leftMargin: 3
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    EditProfileDialog{
        id: myProfileEdit
    }

    onOpened: {
        overlay.opacity = 1
        myProfileWindow.opacity = 1
    }

    onClosed: {
        overlay.opacity = 0
        myProfileWindow.opacity = 0
        login = userlogin
        editButton.visible = true
        myProfileText.text = "My profile";

    }

    function setUserId(userId) {
        user_id = userId;
    }

    function userProfile(userlogin)
    {
        login = userlogin
        editButton.visible = false
        myProfileText.text = "Profile";
    }
}
