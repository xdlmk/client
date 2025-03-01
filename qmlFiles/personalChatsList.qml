import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#17212b"

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
            color: personalChatMouseArea.containsMouse ? "#626a72" : "#1e2a36"
            height: 60
            property int user_id: id
            property string currentState: currentStateText
            property string chatType: currentChatType

            Item {
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
                    anchors{
                        left: parent.left
                        leftMargin: 6
                        top: parent.top
                        topMargin: 6
                    }
                    Image {
                        id:profileImage
                        anchors.fill: parent
                        source: avatarSource + user_id + ".png?" + timestamp
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    id: userLoginId
                    text: userlogin
                    color: "white"
                    font.pointSize: 10
                    font.bold: true
                    anchors{
                        left: personalChatAvatar.right
                        top:parent.top
                        topMargin: 5
                        leftMargin: 10
                    }
                }

                Text {
                    id: messageText
                    anchors{
                        left: personalChatAvatar.right
                        leftMargin: 10
                        top: userLoginId.bottom
                        topMargin: 10
                    }
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
                    upLine.currentState = chatType;

                    for (var i = 0; i < personalChatsListModel.count; ++i) {
                        var item = personalChatsListModel.get(i);
                        if (item.currentStateText === "active") {
                            item.currentStateText = "static";
                            item.color = "#1e2a36";
                            personalChatsListView.forceLayout();
                        }
                    }
                    personalChatsListModel.setProperty(index,"currentStateText","active");
                    if(currentStateText === "active") personalChat.color = "#2b5278";

                    upLine.user_id = user_id;
                    nameText.text = userlogin;
                    changeReceiverUserSignal(userlogin,user_id);
                }
                onEntered: {
                    if (currentStateText === "static"){
                        personalChat.color = "#626a72";
                    }
                }
                onExited: {
                    if (currentStateText === "static"){
                        personalChat.color = "#1e2a36";
                    }
                }
            }

        }
    }
}
