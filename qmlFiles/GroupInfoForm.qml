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
        id:editButton
        text: "\u270E"
        color: "White"
        font.pointSize: 15
        font.bold: false

        anchors.right: closeButton.left
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        visible: false
        MouseArea {
            id: editButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {

            }
        }
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

    Text{
        id:informationIcon
        text: "ⓘ"
        font.pointSize: 18
        color: "White"
        font.bold: true
        anchors.left: membersListView.left
        anchors.top: defLine.bottom
        anchors.topMargin: 12
    }
    ListView {
        id: membersListView
        width: parent.width
        height: parent.height
        anchors.top: informationIcon.bottom
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
            Image{

            }
            Text {
                text: model.username
                color: "White"
                font.pointSize: 14
                anchors.left: parent.left //image
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
            }
            Text{
                id:deleteUserButton
                text: "✕"
                color: "White"
                font.pointSize: 15
                font.bold: true
                visible: activeUserId === creator_id

                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                MouseArea {
                    id: deleteUserButtonMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {

                    }
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
        client.getGroupMembers(group_id,group_name);
    }

    onClosed: {
        overlay.opacity = 0
        groupInfoForm.opacity = 0
    }

    function onLoadGroupMembers(jsonArray) {
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

    Component.onCompleted: {
        loadGroupMembers.connect(onLoadGroupMembers);
    }
}
