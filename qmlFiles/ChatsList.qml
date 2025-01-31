import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: centerLine
    color: "#17212b"
    border.color: "black"
    border.width: 0.5
    anchors.left:  parent.left
    anchors.leftMargin: 54
    anchors.bottom: parent.bottom
    anchors.top: parent.top
    TextArea{
        id: searchField
        selectByMouse: true

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        height: 35

        placeholderText: "Поиск..."
        placeholderTextColor: "grey"
        font.pointSize: 10
        color: "white"
        verticalAlignment: Text.AlignVCenter
        z:2
        background: Rectangle {
            color: "#242f3d"
            radius: searchField.height / 2
        }

        Keys.onReturnPressed: {
            event.accepted = true;
            if (searchField.text.trim() !== "") {
                console.log("Search text: " + searchField.text);
                userSearchListModel.clear();
                sendSearchToServer(searchField.text);
                usersSearchListContainer.z = 5;
                globalMouseArea.z = 6;
            }
        }
    }

    Rectangle {
        id: usersSearchListContainer
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.top: searchField.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        z:2
        color: "#17212b"

        ListView {
            id: userSearchListView
            width: parent.width
            height: parent.height
            anchors.fill: parent
            spacing: 5
            boundsBehavior: Flickable.StopAtBounds

            model: userSearchListModel
            delegate: Rectangle {
                id:searchUser
                width: userSearchListView.width
                color: "#1e2a36"
                height: 60
                z:2
                property int user_id: id
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10

                    Rectangle {
                        id:userAvatar
                        width: 48
                        height: 48
                        radius: 24
                        color: "transparent"
                        border.color: "lightblue"
                        clip: true
                        Image {
                            anchors.fill: parent
                            source: avatarSource
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    Text {
                        text: userlogin
                        color: "white"
                        font.pointSize: 10
                        font.bold: true
                        anchors.left: userAvatar.right
                        anchors.top:parent.top
                        anchors.topMargin: 10
                        anchors.leftMargin: 10
                    }
                }
                MouseArea {
                    id: userSearchMouseArea
                    anchors.fill: parent
                    z:3
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        console.log("Clicked on "+ userlogin + " ID: " + user_id);
                        upLine.currentState = "personal";
                        upLine.user_id = user_id;
                        nameText.text = userlogin;
                        console.log("ChatsList.qml::116 upLine.user_id = " + upLine.user_id + " user id: " + user_id);
                        listModel.clear();
                        usersSearchListContainer.z = 0;
                        changeReceiverUserSignal(userlogin,user_id);
                    }

                    onEntered: {
                        searchUser.color = "#626a72";
                    }

                    onExited: {
                        searchUser.color = "#1e2a36";
                    }
                }

            }
        }
    }

    ListModel {
        id: userSearchListModel
    }

    Rectangle {
        id: personalChatsContainer
        anchors.left: parent.left
        anchors.leftMargin: 1
        anchors.right: parent.right
        anchors.rightMargin: 1
        anchors.top: searchField.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        color: "#17212b"
        z:4

        ListView {
            id: personalChatsListView
            width: parent.width
            height: parent.height
            anchors.fill: parent
            spacing: 5
            boundsBehavior: Flickable.StopAtBounds

            model: personalChatsListModel
            delegate: Rectangle {
                id:personalChat
                width: personalChatsListView.width
                color: "#1e2a36"
                height: 60
                property int user_id: id
                property string currentState: currentStateText
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10

                    Rectangle {
                        id:personalChatAvatar
                        width: 48
                        height: 48
                        radius: 24
                        color: "transparent"
                        border.color: "lightblue"
                        clip: true
                        Image {
                            anchors.fill: parent
                            source: avatarSource
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    Text {
                        id: userLoginId
                        text: userlogin
                        color: "white"
                        font.pointSize: 10
                        font.bold: true
                        anchors.left: personalChatAvatar.right
                        anchors.top:parent.top
                        anchors.topMargin: 5
                        anchors.leftMargin: 10
                    }

                    Text {
                        anchors.left: personalChatAvatar.right
                        anchors.leftMargin: 10
                        anchors.top: userLoginId.bottom
                        anchors.topMargin: 10
                        id: messageText
                        text: message.length > 15 ? message.substring(0, 15) + "..." : message
                        font.pointSize: 10
                        color: "white"
                    }
                }
                MouseArea {
                    id: personalChatMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        console.log("Clicked on "+ userlogin + " ID: " + user_id);
                        upLine.currentState = "personal";

                        for (var i = 0; i < personalChatsListModel.count; ++i) {
                            var item = personalChatsListModel.get(i);
                            //console.log(i + " " + item.currentStateText);
                            //console.log(personalChatsListModel.get(i).color);
                            if (item.currentStateText === "active") {
                                //console.log("Item found");
                                //personalChatsListModel.setProperty(i,"color","#1e2a36");
                                //console.log(item.color);
                                //personalChatsListModel.setProperty(i,"currentState","default");
                                item.currentStateText = "static";
                                item.color = "#1e2a36";
                                //console.log(personalChatsListModel.get(i).color);
                                personalChatsListView.forceLayout();
                            }
                        }

                        //personalChatsListModel.setProperty(index,"color","#2b5278");
                        personalChatsListModel.setProperty(index,"currentStateText","active");
                        if(currentStateText === "active") personalChat.color = "#2b5278";
                        //personalChat.currentStateText = "active";


                        upLine.user_id = user_id;
                        nameText.text = userlogin;
                        console.log("ChatsList.qml::233 upLine.user_id = " + upLine.user_id + " user id: " + user_id);
                        changeReceiverUserSignal(userlogin,user_id);
                    }


                    onEntered: {
                        //console.log("Entered:" + index + " CurrentState = " + currentStateText);
                        if (currentStateText === "static"){
                            personalChat.color = "#626a72";
                        }
                    }

                    onExited: {
                        //console.log("Exited:" + index + " CurrentState = " + currentStateText);
                        if (currentStateText === "static"){
                            personalChat.color = "#1e2a36";
                        }
                    }
                }

            }
        }
    }

    ListModel{
        id:personalChatsListModel
    }

    function onNewSearchUser(userlogin,id)
    {
        var newSearchUsr = {"userlogin":userlogin,"id":id};
        console.log(userlogin + " " + id);
        userSearchListModel.append(newSearchUsr);
        userSearchListView.positionViewAtIndex(userSearchListModel.count - 1, ListView.End);
    }

    function onShowPersonalChat(userlogin,message,id,out)
    {
        console.log(userlogin + " " + message + " "+ id + " "+ out);

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
        if(out === "out")
        {
            newPersChat = {"userlogin":userlogin,"currentStateText": "static", "message": "You: " + message , "id":id};
        }
        else {
            newPersChat = {"userlogin":userlogin,"currentStateText": "static", "message": message , "id":id};
        }

        console.log(userlogin + " " + message + " "+ id);
        personalChatsListModel.insert(0,newPersChat);
    }

    Component.onCompleted: {
        newSearchUser.connect(onNewSearchUser);
        showPersonalChat.connect(onShowPersonalChat);
    }
}
