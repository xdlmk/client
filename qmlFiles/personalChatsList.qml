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
        clip: true

        model: personalChatsListModel
        delegate: Rectangle {
            id:personalChat
            width: personalChatsListView.width
            color: upLine.user_id === user_id ? "#2b5278" : (personalChatMouseArea.containsMouse ? "#626a72" : "#1e2a36")
            height: 60
            property int user_id: id
            property string chatType: currentChatType

            Item {
                anchors.fill: parent
                anchors.leftMargin: 10
                SmartImage {
                    id:personalChatAvatar
                    width: 48
                    height: 48
                    anchors{
                        left: parent.left
                        leftMargin: 6
                        top: parent.top
                        topMargin: 6
                    }
                    textImage: userLogin.text
                    source: (chatType == "group" ? groupAvatarSource : avatarSource) + user_id + ".png?" + timestamp
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: userLogin
                    text: userlogin
                    color: "white"
                    font.pointSize: 10
                    font.bold: true
                    anchors {
                        left: personalChatAvatar.right
                        top:parent.top
                        topMargin: 5
                        leftMargin: 10
                    }
                }

                Text {
                    id: messageText
                    anchors {
                        left: personalChatAvatar.right
                        leftMargin: 10
                        top: userLogin.bottom
                        topMargin: 10
                    }
                    elide: Text.ElideRight
                    width: personalChat.width - (personalChatAvatar.width + personalChatAvatar.anchors.leftMargin + 20)
                    text: message.indexOf('\n') !== -1
                          ? message.substring(0, message.indexOf('\n')) + "..."
                          : message;

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
                    valueText.visible = false;
                    upLine.currentState = chatType;

                    if(user_id !== 0) {
                        upLine.user_id = user_id;
                        nameText.text = userlogin;
                        client.loadingChat(user_id, chatType);

                        if(chatType === "group") {
                            client.getGroupMembers(user_id);
                        }
                    }
                }
            }

        }
    }
}
