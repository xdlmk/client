import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0

    background: Rectangle {
        color: "#1e2a36"
        radius: 6
        border.color: "#626a72"
        border.width: 1/2
    }
    width: 400
    height: 500

    property int group_id: group_id
    property string group_name: group_name
    property int creator_id: creator_id


    Text{
        id:groupInfoText
        text: "Group info"
        color: "White"
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
        color: "White"
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

    Rectangle {
        id:groupAvatar
        width: 80
        height: 80
        radius: 40
        anchors.top: groupInfoText.bottom
        anchors.topMargin: 30
        anchors.left: groupInfoText.left
        color: "transparent"
        border.color: "lightblue"
        clip: true
        Image {
            anchors.fill: parent
            source: group_id !== 0 ? groupAvatarSource + group_id + ".png?" + timestamp : ""
            visible: source !== ""
            fillMode: Image.PreserveAspectFit
        }
    }

    Text{
        id:groupNameText
        text: group_name
        font.pointSize: 12
        color: "White"
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
        color:"#626a72"
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
            color: "White"
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
            onEntered: {addMembersButton.color = "#626a72"}
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
            width: membersListView.width
            height: 40
            color: "#626a72"
            Image {
                id:profileImage
                width: 30
                height: 30
                source: avatarSource + model.user_id + ".png?" + timestamp
                anchors {
                    left: parent.left
                    leftMargin: 5
                    verticalCenter: parent.verticalCenter
                }

            }
            Text {
                text: model.username
                color: "White"
                font.pointSize: 14
                anchors.left: profileImage.right
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
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
            Rectangle{
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
                    color: "White"
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
