import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id:groupInfoFormDialogRoot
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

    property int group_id: group_id
    property string group_name: group_name
    property int creator_id: creator_id

    property string newGroupAvatarPath: ""


    Text{
        id:groupInfoText
        text: "Group info"
        color: isColorLight(groupInfoFormDialogRoot.background.color) ? "black" : "white"
        font.pointSize: 15
        font.bold: true

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    Text{
        id:closeButton
        text: "✕"
        color: isColorLight(groupInfoFormDialogRoot.background.color) ? "black" : "white"
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
                groupInfoForm.close();
            }
        }
    }

    SmartImage {
        id:groupAvatar
        width: 80
        height: 80
        anchors {
            top: groupInfoText.bottom
            topMargin: 30
            left: groupInfoText.left
        }
        textImage: groupNameText.text
        source: group_id !== 0 ? groupAvatarSource + group_id + ".png?" + timestamp : ""
        visible: source !== ""
        fillMode: Image.PreserveAspectFit
    }
    Rectangle {
        id:changeGroupAvatarButton
        width: 20
        height: 20
        radius: 10
        visible: activeUserId === creator_id
        anchors{
            right: groupAvatar.right
            bottom: groupAvatar.bottom
        }
        color: adjustColor(themeManager.chatBackground, 1.5, false)
        border.color: themeManager.outgoingColor
        border.width: 1/2
        Text {
            text: "\u270E"
            color: themeManager.outgoingColor
            font.pointSize: 12
            font.bold: true
            anchors.centerIn: parent
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                newGroupAvatarPath = fileManager.openFile("Image");
                if (newGroupAvatarPath != "") {
                    client.sendNewAvatar(newGroupAvatarPath, "group", group_id);
                    newGroupAvatarPath = "";
                }
            }
        }
    }

    Text{
        id:groupNameText
        text: group_name
        font.pointSize: 12
        color: isColorLight(groupInfoFormDialogRoot.background.color) ? "black" : "white"
        font.bold: true
        anchors.left: groupAvatar.right
        anchors.leftMargin: 12
        anchors.top: groupAvatar.top
        anchors.topMargin: 12
    }

    Rectangle{
        id:defLine
        height: 6
        width:parent.width + 22
        color: adjustColor(themeManager.chatBackground, 1.8, false)
        anchors.left: parent.left
        anchors.leftMargin: -11
        anchors.top: groupAvatar.bottom
        anchors.topMargin: 24
    }

    Rectangle {
        id:addMembersButton
        width: parent.width
        height: 40
        color: "transparent"
        anchors {
            left: membersListView.left
            top: defLine.bottom
            topMargin: 12
        }
        Text{
            anchors {
                verticalCenter: parent.verticalCenter
                left:parent.left
                leftMargin: 10
            }
            text: "Add members"
            font.pointSize: 14
            color: isColorLight(groupInfoFormDialogRoot.background.color) ? "black" : "white"
            font.bold: true
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                selectContactsForm.setParams("add");
                selectContactsForm.open();
            }
            onEntered: {addMembersButton.color = adjustColor(themeManager.chatBackground, 1.75, false)}
            onExited: {addMembersButton.color = "transparent"}
        }

    }
    ListView {
        id: membersListView
        width: parent.width
        height: parent.height
        anchors.top: addMembersButton.bottom
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        model: membersModel
        delegate: memberDelegate
    }

    Component {
        id: memberDelegate

        Rectangle {
            id: memberRect
            width: membersListView.width
            height: 40
            color: adjustColor(themeManager.chatBackground, 1.75, false)
            SmartImage {
                id:profileImage
                width: 30
                height: 30
                textImage: model.username
                source: avatarSource + model.user_id + ".png?" + timestamp
                anchors {
                    left: parent.left
                    leftMargin: 5
                    verticalCenter: parent.verticalCenter
                }
            }
            Text {
                text: model.username
                color: isColorLight(memberRect.color) ? "black" : "white"
                font.pointSize: 14
                anchors {
                    left: profileImage.right
                    leftMargin: 10
                    verticalCenter: parent.verticalCenter
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    overlay.visible = true
                    myProfileWindow.setUserId(model.user_id);
                    myProfileWindow.open();
                    if(activeUserId !== model.user_id){
                        myProfileWindow.userProfile(model.username)
                    }
                }
            }
            Rectangle {
                width: 25
                height: 25
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                visible: activeUserId === creator_id && activeUserId !== model.user_id
                color: "transparent"
                Text{
                    id:deleteUserButton
                    text: "✕"
                    color: isColorLight(memberRect.color) ? "black" : "white"
                    font.pointSize: 15
                    font.bold: true
                    visible: activeUserId === creator_id && activeUserId !== model.user_id
                    anchors.centerIn: parent.Center
                }
                MouseArea {
                    id: deleteUserButtonMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        client.deleteMemberFromGroup(model.user_id, group_id);
                    }
                }
            }
        }
    }

    ListModel {
        id: membersModel
    }

    function setGroupId(groupId) {
        group_id = groupId;
    }

    function setGroupName(groupName)
    {
        group_name = groupName
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    onOpened: {
        overlay.opacity = 1
        groupInfoForm.opacity = 1
        client.getGroupMembers(group_id);
    }

    onClosed: {
        overlay.opacity = 0
        groupInfoForm.opacity = 0
    }
    function isMemberExists(userId) {
        for (var i = 0; i < membersModel.count; i++) {
            var member = membersModel.get(i)
            if (member.user_id === userId) {
                return true
            }
        }
        return false
    }

    function onLoadGroupMembers(jsonArray, group_id) {
        if(groupInfoForm.group_id === group_id){
            membersModel.clear();
            for (var i = 0; i < jsonArray.length; i++) {
                var member = jsonArray[i];
                membersModel.append({ "user_id": member.id, "username": member.username, "status": member.status });
                client.checkAndSendAvatarUpdate(member.avatar_url, member.id,"personal");
                if(member.status === "creator"){
                    creator_id = member.id;
                }
            }
        }
    }

    Component.onCompleted: {
        loadGroupMembers.connect(onLoadGroupMembers);
    }
}
