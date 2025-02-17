import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: centerLine
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

    ListModel{
        id:personalChatsListModel
    }

    function onShowPersonalChat(userlogin,message,id,out)
    {
        var exists = false;
        for (var i = 0; i < personalChatsListModel.count; i++) {
            var item = personalChatsListModel.get(i);
            if (item.id === id) {
                exists = true;
                personalChatsListModel.remove(i);
                break;
            }
        }
        var newPersChat;
        if(out === "out") {
            newPersChat = {"userlogin":userlogin,"currentStateText": "static", "message": "You: " + message , "id":id};
        }
        else {
            newPersChat = {"userlogin":userlogin,"currentStateText": "static", "message": message , "id":id};
        }
        personalChatsListModel.insert(0,newPersChat);
    }

    Component.onCompleted: {
        showPersonalChat.connect(onShowPersonalChat);
    }
}
