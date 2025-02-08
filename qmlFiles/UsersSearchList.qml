import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
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
            color: userSearchMouseArea.containsMouse ? "#626a72" : "#1e2a36"
            height: 60
            property int user_id: id
            Item {
                id:userSearchContainer
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
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true

                onClicked: {
                    console.log("Clicked on "+ userlogin + " ID: " + user_id);
                    upLine.currentState = "personal";
                    upLine.user_id = user_id;
                    nameText.text = userlogin;
                    listModel.clear();
                    isSearchListExtended = false;
                    console.log(isSearchListExtended);
                    searchField.clear();
                    changeReceiverUserSignal(userlogin,user_id);
                }
            }

        }
    }

    function onNewSearchUser(userlogin,id)
    {
        var newSearchUsr = {"userlogin":userlogin,"id":id};
        console.log(userlogin + " " + id);
        userSearchListModel.append(newSearchUsr);
        userSearchListView.positionViewAtIndex(userSearchListModel.count - 1, ListView.End);
    }

    Component.onCompleted: {
        newSearchUser.connect(onNewSearchUser);
    }
}
