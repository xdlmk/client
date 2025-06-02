import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: adjustColor(themeManager.chatBackground, 1.5, false)
    property alias personalChatsListView: personalChatsListView

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
            color: upLine.user_id === user_id ? themeManager.outgoingColor : (personalChatMouseArea.containsMouse ? adjustColor(themeManager.chatBackground, 1.75, false) : themeManager.incomingColor)
            height: 60
            property int user_id: id
            property string chatType: currentChatType
            property string timestamp: messageTimestamp
            property int unreadMessagesCount: unreadCount

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
                    color: isColorLight(personalChat.color) ? "black" : "white"
                    font.pointSize: 10
                    font.bold: true
                    anchors {
                        left: personalChatAvatar.right
                        top:parent.top
                        topMargin: 5
                        leftMargin: 10
                    }
                    elide: Text.ElideRight
                    width: personalChat.width - (personalChatAvatar.width + personalChatAvatar.anchors.leftMargin + 20 + lblTime.width + lblTime.anchors.rightMargin)
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
                    width: personalChat.width - (personalChatAvatar.width + personalChatAvatar.anchors.leftMargin + lblUnreadCount.width + lblUnreadCount.anchors.rightMargin + 20)
                    text: message.indexOf('\n') !== -1
                          ? message.substring(0, message.indexOf('\n')) + "..."
                          : message;

                    font.pointSize: 10
                    color: isColorLight(personalChat.color) ? "black" : "white"
                }

                Text {
                    id: lblTime
                    anchors{
                        right: parent.right
                        top: parent.top
                        rightMargin: 5
                        topMargin: 5
                    }
                    text: extractTimeFromTimestamp(timestamp)
                    font.pointSize: 8
                    color: isColorLight(personalChat.color) ? "darkgrey" : "grey"
                    horizontalAlignment: Text.AlignRight
                }

                Rectangle {
                    id: lblUnreadCount
                    visible: unreadMessagesCount !== 0
                    color: themeManager.outgoingColor
                    width: textItem.implicitWidth + 10
                    height: width
                    radius: width / 2
                    anchors {
                        right: lblTime.right
                        rightMargin: 5
                        verticalCenter: messageText.verticalCenter
                    }

                    Text {
                        id: textItem
                        anchors.centerIn: parent
                        text: unreadMessagesCount > 99 ? "99+" : unreadMessagesCount
                        font.pointSize: 8
                        font.bold: true
                        color: isColorLight(lblUnreadCount.color) ? "black" : "white"
                    }
                }
            }
            MouseArea {
                id: personalChatMouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true

                onClicked: {
                    if(!(upLine.user_id === user_id && upLine.currentState === chatType)) {
                        valueText.visible = false;
                        upLine.currentState = chatType;

                        if(user_id !== 0) {
                            upLine.user_id = user_id;
                            nameText.text = userlogin;
                            downLine.clearData();
                            client.loadingChat(user_id, chatType);

                            if(chatType === "group") {
                                client.getGroupMembers(user_id);
                            }
                        }
                    }
                }
            }

            function countUnreadMessages() {
                var unreadCount = 0;
                for (var i = listModel.count - 1; i >= 0; i--) {
                    var message = listModel.get(i);
                    if (message.isOutgoing === true)
                        break;
                    if (message.isRead === true)
                        break;
                    unreadCount++;
                }
                unreadMessagesCount = unreadCount;
            }
        }
    }



    function extractTimeFromTimestamp(timestamp) {
        var date = new Date(timestamp);
        var hours = date.getUTCHours();
        var minutes = date.getUTCMinutes();
        if (hours < 10)
            hours = "0" + hours;
        if (minutes < 10)
            minutes = "0" + minutes;
        return hours + ":" + minutes;
    }
}
