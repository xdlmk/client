import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Dialog {
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0

    background: Rectangle {
        color: "#1e2a36"
        radius: 6
        border.color: "#626a72"
        border.width: 1/2
    }
    width: 400
    height: 500

    Text{
        id:information
        text: "Information"
        color: "White"
        font.pointSize: 15
        font.bold: true

        anchors.left: backButton.right
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    Text{
        id:backButton
        text: "\u27F5"
        color: "White"
        font.pointSize: 15
        font.bold: false

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        MouseArea {
            id: backButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                myProfileEdit.close()
            }
        }
    }

    Text{
        id:closeButton
        text: "✕"
        color: "White"
        font.pointSize: 15
        font.bold: true

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        MouseArea {
            id: closeButtonMouseArea
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
        anchors.top: information.bottom
        anchors.topMargin: 12
        anchors.left: information.left
        Rectangle {
            id:userAvatar
            width: 120
            height: 120
            radius: 60
            anchors.left: parent.left
            anchors.leftMargin: (parent.width - width)/4
            anchors.top: parent.top
            color: "transparent"
            border.color: "lightblue"
            clip: true
            Image {
                anchors.fill: parent
                source: avatarSource
                fillMode: Image.PreserveAspectFit
            }
        }
        Text{
            id:userLoginText
            text: userlogin
            font.pointSize: 12
            color: "White"
            font.bold: true
            anchors.left: userAvatar.left
            anchors.leftMargin: (userAvatar.width - width)/2
            anchors.top: userAvatar.bottom
            anchors.topMargin: 12
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
        background: Rectangle {
            color: root.color
        }
    }

    EditFiledInformation{
        id:editInformation
    }

    Rectangle{
        id:defLine
        height: 6
        width:parent.width + 22
        color:"#626a72"
        anchors.left: parent.left
        anchors.leftMargin: -11
        anchors.top: aboutMe.bottom
        anchors.topMargin: 24
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
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.topMargin: 3

                Rectangle {
                    id:icon
                    width: 24
                    height: 24
                    radius: 12
                    color: "transparent"
                    border.color: "lightblue"
                    clip: true
                    anchors.top: parent.top
                    anchors.topMargin: (parent.height - height)/2
                    anchors.left: parent.left
                    anchors.leftMargin: 3
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
                    anchors.left: icon.right
                    anchors.top:icon.top
                    anchors.leftMargin: 10
                }
                Text{
                    id: informationGet
                    text: information
                    font.pointSize: 12
                    color: "#0078D4"
                    anchors.top: icon.top
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    //textFormat: Text.RichText
                }
            }
            MouseArea {
                id: informationFieldMouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true

                onClicked: {
                    editInformation.infoType = informationName;
                    editInformation.open();
                }

                onEntered: {
                    informationField.color = "#626a72";
                }

                onExited: {
                    informationField.color = "#1e2a36";
                }
            }
        }
    }

    ListModel {
        id: informationListModel
        ListElement {
            iconSource: ""
            informationName: "Name"
            information: "xdlmq"
        }
        ListElement {
            iconSource: ""
            informationName: "Phone number"
            information: "phone number"
        }
        ListElement {
            iconSource: ""
            informationName: "Username"
            information: "username"
        }
    }

    onOpened: {
        myProfileWindow.opacity = 0
        myProfileEdit.opacity = 1

    }

    onClosed: {
        myProfileWindow.opacity = 1
        myProfileEdit.opacity = 0
    }
}
