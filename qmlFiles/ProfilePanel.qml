import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: profileWindow
    width: 250
    height: parent.height
    color: "#1e2a36"
    border.color: "black"
    anchors.top: parent.top
    anchors.bottom: parent.bottom

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            profileImage.source = avatarSource + activeUserId + ".png?" + new Date().getTime();
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
            color: "transparent"
            anchors.left: parent.left
            anchors.leftMargin: 10
            clip: true
            Image {
                id: profileImage
                source: avatarSource + activeUserId + ".png?" + new Date().getTime()
                width: parent.width
                height: parent.height
                cache: false
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
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
            id: usersListContainer
            width: parent.width
            height: Math.min(userListModel.count, 3) * 35 + 10
            color: "#1e2a36"

            ListView {
                id: userListView
                width: parent.width
                height: parent.height
                anchors.top: parent.top
                anchors.topMargin: 10
                spacing: 5
                boundsBehavior: Flickable.StopAtBounds

                model: userListModel
                delegate: Rectangle {
                    id:userChange
                    width: userListView.width
                    color: "#1e2a36"
                    height: 30
                    property int user_id: id

                    Item {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.topMargin: 3

                        Rectangle {
                            id:userAvatar
                            width: 24
                            height: 24
                            radius: 12
                            color: "transparent"
                            border.color: "lightblue"
                            clip: true
                            Image {
                                anchors.fill: parent
                                source: avatarSource + user_id + ".png"
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        Text {
                            text: username
                            color: "white"
                            font.pointSize: 10
                            font.bold: true
                            anchors.left: userAvatar.right
                            anchors.top:parent.top
                            anchors.topMargin: 3
                            anchors.leftMargin: 10
                        }
                    }
                    MouseArea {
                        id: userChangeMouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true

                        onClicked: {
                            if(!isSpecial)
                            {
                                if (username != userlogin)
                                {
                                    client.changeActiveAccount(username);
                                }
                            }
                            else
                            {
                                client.addAccount();
                            }
                        }

                        onEntered: {
                            userChange.color = "#626a72";
                        }

                        onExited: {
                            userChange.color = "#1e2a36";
                        }
                    }
                }
            }

            ListModel {
                id: userListModel
            }


            Rectangle{
                id:upLine
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: "black"
            }
            Rectangle{
                id:downLine
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "black"
            }
        }


        Rectangle{
            id:settings
            anchors.left:  parent.left
            anchors.right: parent.right
            height:100
            y:100
            color: "#1e2a36"

            Rectangle {
                id:openMyProfileButton
                width: parent.width
                color: "#1e2a36"
                height: 30

                Item {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.topMargin: 3

                    Rectangle {
                        id:myProfileImage
                        width: 24
                        height: 24
                        radius: 12
                        color: "transparent"
                        border.color: "lightblue"
                        clip: true
                        Image {
                            anchors.fill: parent
                            //source: myProfileButtonImage
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    Text {
                        text: "My profile"
                        color: "white"
                        font.pointSize: 10
                        font.bold: true
                        anchors.left: myProfileImage.right
                        anchors.top: parent.top
                        anchors.topMargin: 3
                        anchors.leftMargin: 10
                    }
                }
                MouseArea {
                    id: myProfileButtonMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        isProfileExtended = false
                        overlay.visible = true
                        myProfileWindow.open()
                    }

                    onEntered: {
                        openMyProfileButton.color = "#626a72";
                    }

                    onExited: {
                        openMyProfileButton.color = "#1e2a36";
                    }
                }
            }
            Button{
                id:setti2
                anchors.top:openMyProfileButton.bottom
                text:"Settings2"
            }
            Button{
                id:setti3
                anchors.top:setti2.bottom
                text:"Settings3"
            }
            Button{
                id:setti4
                anchors.top:setti3.bottom
                text:"Settings4"
            }
        }

        Rectangle {
            id: buttonLeave
            color: "#1e2a36"
            height: 40
            anchors.left:  parent.left
            anchors.right: parent.right

            Text {
                id: leaveText
                text: "Leave"
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
                    client.clientLogout();
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

    function onNewUser(name) {
        var newUsr = {"username":name,"isSpecial": false};
        userListModel.append(newUsr);
        userListView.positionViewAtIndex(userListModel.count - 1, ListView.End);


        var hasAddAccount = false;
        for (var i = 0; i < userListModel.count; i++) {
            if (userListModel.get(i).isSpecial) {
                hasAddAccount = true;
                userListModel.move(i, userListModel.count - 1,1);
                break;
            }
        }

        if (userListModel.count < 3 && !hasAddAccount ) {
            var specUsr = {"username":"Add account","isSpecial": true};
            userListModel.append(specUsr);
            userListView.positionViewAtIndex(userListModel.count, ListView.End);
        }

        if (userListModel.count > 3 && hasAddAccount) {
            for (var j = 0; j < userListModel.count; j++) {
                if (userListModel.get(j).isSpecial) {
                    userListModel.remove(j);
                    break;
                }
            }
        }

    }

    function clearUserList() {
        userListModel.clear();
    }

    Component.onCompleted: {
        newUser.connect(onNewUser);
        clearUserListModel.connect(clearUserList);
    }


}
