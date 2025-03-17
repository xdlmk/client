import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Dialog {
    id: myProfileEdit
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0
    background: Rectangle {
        color: "#1e2a36"
        radius: 6
        border.color: "#626a72"
        border.width: 0.3
    }
    width: 400
    height: 500
    property string login: userlogin
    property string newAvatarPath: ""

    Text{
        id:information
        text: "Information"
        color: "White"
        font.pointSize: 15
        font.bold: true
        anchors{
            left: backButton.right
            leftMargin: 10
            top: parent.top
            topMargin: 10
        }
    }

    Text{
        id:backButton
        text: "\u27F5"
        color: "White"
        font.pointSize: 15
        font.bold: false
        anchors{
            left: parent.left
            leftMargin: 10
            top: parent.top
            topMargin: 10
        }
        MouseArea {
            id: backButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: { myProfileEdit.close() }
        }
    }

    Text{
        id:closeButton
        text: "✕"
        color: "White"
        font.pointSize: 15
        font.bold: true
        anchors{
            right: parent.right
            rightMargin: 10
            top: parent.top
            topMargin: 10
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                myProfileEdit.close()
                myProfileWindow.close()
            }
        }
    }

    Item{
        id:userSourceContainer
        width: 400
        height: 160
        anchors{
            top: information.bottom
            topMargin: 12
            left: information.left
        }
        Rectangle {
            id:userAvatar
            width: 120
            height: 120
            radius: 60
            anchors{
                left: parent.left
                leftMargin: (parent.width - width)/4
                top: parent.top
            }
            color: "transparent"
            border.color: "lightblue"
            clip: true
            SmartImage {
                anchors.fill: parent
                textImage: userLoginText.text
                source: avatarSource + activeUserId + ".png?" + timestamp
                fillMode: Image.PreserveAspectFit
            }
        }
        Rectangle {
            id:changeUserAvatarButton
            width: 30
            height: 30
            radius: 15
            anchors{
                right: userAvatar.right
                bottom: userAvatar.bottom
            }
            color: "#2b5278"
            border.color: "#182533"
            Text {
                text: "\u270E"
                color:"#182533"
                font.pointSize: 15
                font.bold: true
                anchors.centerIn: parent
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    newAvatarPath = fileManager.openFile("Image");
                    if (newAvatarPath != "") {
                        client.sendNewAvatar(newAvatarPath, "personal", activeUserId);
                        newAvatarPath = "";
                    }
                }
            }
        }
        Text{
            id:userLoginText
            text: userlogin
            font.pointSize: 12
            color: "White"
            font.bold: true
            anchors{
                left: userAvatar.left
                leftMargin: (userAvatar.width - width)/2
                top: userAvatar.bottom
                topMargin: 12
            }
        }
    }



    TextField{
        id:aboutMe
        width:parent.width
        selectByMouse: true
        placeholderText: "About me"
        placeholderTextColor: "grey"
        anchors.top: userSourceContainer.bottom
        font.pointSize: 10
        color: "white"
        background: Rectangle { color: root.color }
    }

    EditFiledInformation{ id:editInformation }

    Rectangle{
        id:defLine
        height: 6
        width:parent.width + 22
        color:"#626a72"
        anchors{
            left: parent.left
            leftMargin: -11
            top: aboutMe.bottom
            topMargin: 24
        }
    }

    ListView {
        id: informationListView
        width: parent.width
        height: parent.height
        anchors.top: defLine.bottom
        anchors.topMargin: 10
        spacing: 5
        boundsBehavior: Flickable.StopAtBounds
        model: informationListModel
        delegate: Rectangle {
            id: informationField
            width: informationListView.width
            color: "#1e2a36"
            height: 40
            property string iconSource: model.iconSource
            property string informationName: model.informationName
            property string information: model.information

            Item {
                anchors{
                    fill: parent
                    leftMargin: 10
                    topMargin: 3
                }
                Rectangle {
                    id:icon
                    width: 24
                    height: 24
                    radius: 12
                    color: "transparent"
                    border.color: "lightblue"
                    clip: true
                    anchors{
                        top: parent.top
                        topMargin: (parent.height - height)/2
                        left: parent.left
                        leftMargin: 3
                    }
                    Image {
                        anchors.fill: parent
                        source: iconSource
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    text: informationName
                    color: "white"
                    font.pointSize: 10
                    font.bold: true
                    anchors{
                        left: icon.right
                        top:icon.top
                        leftMargin: 10
                    }
                }
                Text{
                    id: informationGet
                    text: information
                    font.pointSize: 12
                    color: "#0078D4"
                    anchors{
                        top: icon.top
                        right: parent.right
                        rightMargin: 10
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    editInformation.infoType = informationName;
                    editInformation.open();
                }
                onEntered: { informationField.color = "#626a72"; }
                onExited: { informationField.color = "#1e2a36"; }
            }
        }
    }

    ListModel { id: informationListModel }

    onOpened: {
        myProfileWindow.opacity = 0
        myProfileEdit.opacity = 1
        informationListModel.clear();
        informationListModel.append({ iconSource: "", informationName: "Name", information: "name" });
        informationListModel.append({ iconSource: "", informationName: "Phone number", information: "+810128919" });
        informationListModel.append({ iconSource: "", informationName: "Username", information: login });
    }
    onClosed: {
        myProfileWindow.opacity = 1
        myProfileEdit.opacity = 0
    }
}
