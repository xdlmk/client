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
    width: 300
    height: 200

    property string infoType: "default"

    Text{
        id:information
        text: {
            if(editInformation.infoType === "Name"){
                return "Editing name"
            } else if(editInformation.infoType === "Phone number"){
                return "Editing phone number"
            } else if(editInformation.infoType === "Username"){
                return "Editing username"
            } else if(editInformation.infoType === "default"){
                return ""
            }
        }
        color: "White"
        font.pointSize: 15
        font.bold: true

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    TextField{
        id:editableFiled
        width:parent.width
        selectByMouse: true
        placeholderText: {
            if(editInformation.infoType === "Name"){
                return "Name"
            } else if(editInformation.infoType === "Phone number"){
                return "Phone number"
            } else if(editInformation.infoType === "Username"){
                return "Username"
            } else if(editInformation.infoType === "default"){
                return ""
            }
        }
        placeholderTextColor: "grey"
        anchors.top: information.bottom
        anchors.topMargin: 10 + height
        font.pointSize: 10
        color: "white"
        background: Rectangle {
            color: "transparent"
        }
    }
    Rectangle
    {
        id:recEditField
        color: "#2f6ea5"
        width: parent.width
        height: 2
        anchors.left: editableFiled.left
        anchors.leftMargin: 6
        anchors.top: editableFiled.bottom
    }

    Text{
        id:cansel
        text: "Cansel"
        color: "White"
        font.pointSize: 12
        font.bold: false

        anchors.right: save.left
        anchors.rightMargin: 10
        anchors.bottom: save.bottom
        MouseArea {
            id: closeButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                editInformation.close()
            }
        }
    }

    Text{
        id:save
        text: "Save"
        color: "White"
        font.pointSize: 12
        font.bold: false

        anchors.right: parent.right
        anchors.leftMargin: 10
        anchors.bottom: parent.bottom
        anchors.topMargin: 10
        MouseArea {
            id: saveButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                client.sendEditProfileRequest(editableFiled.placeholderText,editableFiled.text)
                editInformation.close()
                myProfileEdit.close()
                myProfileWindow.close()
            }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    onOpened: {
        editInformation.opacity = 1

    }

    onClosed: {
        editableFiled.clear()
        editInformation.opacity = 0
    }
}
