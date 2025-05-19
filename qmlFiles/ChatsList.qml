import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

Rectangle {
    id: centerLine
    property alias chatsListView: personalChatsContainer.personalChatsListView
    color: "#17212b"
    anchors{
        left:  leftLine.right
        bottom: parent.bottom
        top: parent.top
    }
    TextArea{
        id: searchField
        selectByMouse: true
        anchors{
            right: parent.right
            left: parent.left
            top: parent.top
            margins: 10
        }
        height: 35
        placeholderText: "Search..."
        placeholderTextColor: "grey"
        font.pointSize: 10
        color: "white"
        verticalAlignment: Text.AlignVCenter
        background: Rectangle {
            color: "#242f3d"
            radius: searchField.height / 2
        }
        Keys.onReturnPressed: {
            if (searchField.text.trim() !== "") {
                userSearchListModel.clear();
                sendSearchToServer(searchField.text);
                isSearchListExtended = true;
            }
        }
    }

    UsersSearchList {
        id: usersSearchListContainer
        enabled: isSearchListExtended
        visible: isSearchListExtended

        anchors{
            left: parent.left
            leftMargin: 1
            right: parent.right
            rightMargin: 1
            top: searchField.bottom
            topMargin: 10
            bottom: parent.bottom
        }
    }

    ListModel {
        id: userSearchListModel
    }

    PersonalChatsList {
        id: personalChatsContainer
        enabled: !isSearchListExtended
        visible: !isSearchListExtended

        anchors{
            left: parent.left
            leftMargin: 1
            right: parent.right
            rightMargin: 1
            top: searchField.bottom
            topMargin: 10
            bottom: parent.bottom
        }
    }

    function sortModel() {
        var items = [];
        for (var i = 0; i < personalChatsListModel.count; i++) {
            var element = personalChatsListModel.get(i);
            var newObj = {
                userlogin: element.userlogin || "",
                currentChatType: element.currentChatType || "",
                currentStateText: element.currentStateText || "",
                message: element.message || "",
                id: element.id || "",
                messageTimestamp: element.messageTimestamp || "",
                unreadCount: Number(element.unreadCount) || 0
            };
            items.push(newObj);
        }
        items.sort(function(a, b) {
            return Date.parse(b.messageTimestamp) - Date.parse(a.messageTimestamp);
        });
        personalChatsListModel.clear();
        for (var j = 0; j < items.length; j++) {
            personalChatsListModel.append(items[j]);
        }
    }

    function onShowPersonalChat(userlogin,message,id,out,type, timestamp, unreadCount) {
        var exists = false;
        for (var i = 0; i < personalChatsListModel.count; i++) {
            var item = personalChatsListModel.get(i);
            if (item.id === id && item.currentChatType === type) {
                exists = true;
                personalChatsListModel.remove(i);
                break;
            }
        }
        var newPersChat;
        if(out === "out") {
            newPersChat = {"userlogin":userlogin, "currentChatType":type, "currentStateText": "static", "message": "You: " + message, "id":id, "messageTimestamp": timestamp, "unreadCount": unreadCount};
        }
        else {
            newPersChat = {"userlogin":userlogin, "currentChatType":type, "currentStateText": "static", "message": message , "id":id,  "messageTimestamp": timestamp, "unreadCount": unreadCount};
        }
        personalChatsListModel.insert(0,newPersChat);
        sortModel();
    }

    function removeChatAfterDelete(group_id) {
        var type = "group";
        for (var i = 0; i < personalChatsListModel.count; i++) {
            var item = personalChatsListModel.get(i);
            if (item.id === group_id && item.currentChatType === type) {
                personalChatsListModel.remove(i);
                break;
            }
        }
    }

    Component.onCompleted: {
        showPersonalChat.connect(onShowPersonalChat);
        clearMessagesAfterDelete.connect(removeChatAfterDelete);
    }
}
