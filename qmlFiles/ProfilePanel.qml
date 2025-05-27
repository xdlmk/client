import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Rectangle {
    id: profileWindow
    width: 250
    height: parent.height
    color: themeManager.chatBackground
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    border.color: "black"

    Column {
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 2
        spacing: 10

        SmartImage {
            id: profileImage
            source: avatarSource + activeUserId + ".png?" + timestamp
            width: 50
            height: 50
            textImage: userLoginText.text
            cache: false
            anchors.left: parent.left
            anchors.leftMargin: 10
            smooth: true
            fillMode: Image.PreserveAspectCrop
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
            color: themeManager.chatBackground

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
                    color: themeManager.chatBackground
                    height: 30

                    Item {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.topMargin: 3
                        SmartImage {
                            id: userAvatar
                            visible: !isSpecial
                            width: 24
                            height: 24
                            textImage: username
                            source: isSpecial ? "" : avatarSource + id + ".png?" + timestamp
                            fillMode: Image.PreserveAspectFit
                        }
                        Rectangle {
                            id:addAccountIcon
                            visible: isSpecial
                            width: 24
                            height: 24
                            radius: 12
                            color: themeManager.outgoingColor
                            Text {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    horizontalCenter: parent.horizontalCenter
                                }
                                font.bold: true
                                text: "+"
                                font.pointSize: 14
                                color: "white"
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
                            if(!isSpecial) {
                                if (username != userlogin) {
                                    client.changeActiveAccount(username);
                                }
                            } else {
                                client.addAccount();
                            }
                        }

                        onEntered: {
                            userChange.color = Qt.lighter(themeManager.chatBackground, 1.75);
                        }

                        onExited: {
                            userChange.color = themeManager.chatBackground;
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
            color: themeManager.chatBackground

            Rectangle {
                id:openMyProfileButton
                width: parent.width
                color: themeManager.chatBackground
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
                        color: themeManager.outgoingColor
                        clip: true
                        Image {
                            width: 20
                            height: 20
                            anchors.centerIn: parent
                            source: "../images/myProfileIcon.svg"
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
                        myProfileWindow.setUserId(activeUserId)
                        myProfileWindow.open()
                    }

                    onEntered: {
                        openMyProfileButton.color = Qt.lighter(themeManager.chatBackground, 1.75);
                    }

                    onExited: {
                        openMyProfileButton.color = themeManager.chatBackground;
                    }
                }
            }
            Rectangle {
                id:openGroupCreateButton
                width: parent.width
                anchors.top:openMyProfileButton.bottom
                color: themeManager.chatBackground
                height: 30

                Item {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.topMargin: 3

                    Rectangle {
                        id:groupCreateImage
                        width: 24
                        height: 24
                        radius: 12
                        color: themeManager.outgoingColor
                        clip: true
                        Image {
                            width: 20
                            height: 20
                            anchors.centerIn: parent
                            source: "../images/createGroupIcon.svg"
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    Text {
                        text: "Create group"
                        color: "white"
                        font.pointSize: 10
                        font.bold: true
                        anchors.left: groupCreateImage.right
                        anchors.top: parent.top
                        anchors.topMargin: 3
                        anchors.leftMargin: 10
                    }
                }
                MouseArea {
                    id: groupCreateButtonMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        isProfileExtended = false
                        overlay.visible = true
                        createGroupForm.open()
                    }

                    onEntered: {
                        openGroupCreateButton.color = Qt.lighter(themeManager.chatBackground, 1.75);
                    }

                    onExited: {
                        openGroupCreateButton.color = themeManager.chatBackground;
                    }
                }
            }
            Rectangle {
                id:openDesignChangeButton
                width: parent.width
                anchors.top:openGroupCreateButton.bottom
                color: themeManager.chatBackground
                height: 30

                Item {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.topMargin: 3

                    Rectangle {
                        id: designChangeImage
                        width: 24
                        height: 24
                        radius: 12
                        color: themeManager.outgoingColor
                        clip: true
                        Image {
                            width: 20
                            height: 20
                            anchors.centerIn: parent
                            source: "../images/changeDesignIcon.svg"
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    Text {
                        text: "Design"
                        color: "white"
                        font.pointSize: 10
                        font.bold: true
                        anchors.left: designChangeImage.right
                        anchors.top: parent.top
                        anchors.topMargin: 3
                        anchors.leftMargin: 10
                    }
                }
                MouseArea {
                    id: designChangeButtonMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        overlay.visible = true
                        themeSettings.open()
                    }

                    onEntered: {
                        openDesignChangeButton.color = Qt.lighter(themeManager.chatBackground, 1.75);
                    }

                    onExited: {
                        openDesignChangeButton.color = themeManager.chatBackground;
                    }
                }
            }
        }

        Rectangle {
            id: buttonLeave
            color: themeManager.chatBackground
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
                    buttonLeave.color = Qt.lighter(themeManager.chatBackground, 1.75);
                }

                onExited: {
                    buttonLeave.color = themeManager.chatBackground;
                }
            }
        }
    }

    function onNewUser(name,id) {
        var newUsr = {"username":name,"isSpecial": false, "id": id};
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
