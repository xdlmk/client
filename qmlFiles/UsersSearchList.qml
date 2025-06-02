import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: adjustColor(themeManager.chatBackground, 1.5, false)

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
            color: userSearchMouseArea.containsMouse ? adjustColor(themeManager.chatBackground, 1.75, false) : themeManager.incomingColor
            height: 60
            property int user_id: id
            Item {
                id:userSearchContainer
                anchors.fill: parent
                anchors.leftMargin: 10

                SmartImage {
                    id:userAvatar
                    width: 48
                    height: 48
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                    }
                    textImage: userlogin
                    source: avatarSource + user_id + ".png?" + timestamp
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: userlogin
                    color: isColorLight(searchUser.color) ? "black" : "white"
                    font.pointSize: 10
                    font.bold: true
                    anchors{
                        left: userAvatar.right
                        leftMargin: 10
                        top:parent.top
                        topMargin: 10
                    }
                }
            }
            MouseArea {
                id: userSearchMouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true

                onClicked: {
                    if(!(upLine.user_id === user_id && upLine.currentState === "personal")) {
                        valueText.visible = false;
                        upLine.currentState = "personal";

                        if(user_id !== 0) {
                            upLine.user_id = user_id;
                            nameText.text = userlogin;
                            downLine.clearData();
                            client.loadingChat(user_id, "personal");
                        }
                    }
                    isSearchListExtended = false;
                    searchField.clear();
                }
            }

        }
    }

    function onNewSearchUser(userlogin,id)
    {
        var newSearchUsr = {"userlogin":userlogin,"id":id};
        logger.qmlLog("INFO","UsersSearchList.qml::onNewSearchUser","New search user:" + userlogin + " with id " + id);
        userSearchListModel.append(newSearchUsr);
        userSearchListView.positionViewAtIndex(userSearchListModel.count - 1, ListView.End);
    }

    Component.onCompleted: {
        newSearchUser.connect(onNewSearchUser);
    }
}
