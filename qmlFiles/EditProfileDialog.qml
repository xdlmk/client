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
        color: adjustColor(themeManager.chatBackground, 1.5, false)
        radius: 6
    }
    width: 400
    height: 500
    property string login: userlogin
    property string newAvatarPath: ""

    Text{
        id:information
        text: "Information"
        color: isColorLight(myProfileEdit.background.color) ? "black" : "white"
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
        color: isColorLight(myProfileEdit.background.color) ? "black" : "white"
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
        color: isColorLight(myProfileEdit.background.color) ? "black" : "white"
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
        SmartImage {
            id: userAvatar
            width: 120
            height: 120
            anchors{
                left: parent.left
                leftMargin: (parent.width - width)/4
                top: parent.top
            }
            textImage: userLoginText.text
            source: avatarSource + activeUserId + ".png?" + timestamp
            fillMode: Image.PreserveAspectFit
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
            color: adjustColor(themeManager.chatBackground, 1.5, false)
            border.color: themeManager.outgoingColor
            border.width: 1/2
            Text {
                text: "\u270E"
                color: themeManager.outgoingColor
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
            color: isColorLight(myProfileEdit.background.color) ? "black" : "white"
            font.bold: true
            anchors{
                left: userAvatar.left
                leftMargin: (userAvatar.width - width)/2
                top: userAvatar.bottom
                topMargin: 12
            }
        }
    }

    EditFiledInformation{ id:editInformation }

    Rectangle{
        id:defLine
        height: 6
        width:parent.width + 22
        color: adjustColor(themeManager.chatBackground, 1.8, false)
        anchors{
            left: parent.left
            leftMargin: -11
            top: userSourceContainer.bottom
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
            color: adjustColor(themeManager.chatBackground, 1.5, false)
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
                    clip: true
                    anchors{
                        top: parent.top
                        topMargin: (parent.height - height)/2
                        left: parent.left
                        leftMargin: 3
                    }
                    SmartImage {
                        anchors.fill: parent
                        textImage: informationName
                        source: iconSource
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    text: informationName
                    color: isColorLight(myProfileEdit.background.color) ? "black" : "white"
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
                onEntered: { informationField.color = adjustColor(themeManager.chatBackground, 1.75, false); }
                onExited: { informationField.color = adjustColor(themeManager.chatBackground, 1.5, false); }
            }
        }
    }

    ListModel { id: informationListModel }

    onOpened: {
        myProfileWindow.opacity = 0
        myProfileEdit.opacity = 1
        informationListModel.clear();
        //informationListModel.append({ iconSource: "", informationName: "Name", information: "name" });
        //informationListModel.append({ iconSource: "", informationName: "Phone number", information: "+810128919" });
        informationListModel.append({ iconSource: "", informationName: "Username", information: login });
    }
    onClosed: {
        myProfileWindow.opacity = 1
        myProfileEdit.opacity = 0
    }
}
