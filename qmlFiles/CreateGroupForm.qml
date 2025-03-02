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

    Text{
        id:createGroupText
        text: "Create Group"
        color: "White"
        font.pointSize: 15
        font.bold: true
        anchors{
            left: parent.left
            leftMargin: 10
            top: parent.top
            topMargin: 10
        }
    }

    TextField{
        id:groupName
        width:parent.width
        selectByMouse: true
        placeholderText: "Group name"
        placeholderTextColor: "grey"
        anchors.top: createGroupText.bottom
        anchors.topMargin: 10 + height
        font.pointSize: 10
        color: "white"
        background: Rectangle { color: "transparent" }
        onTextChanged: {
            if (text.length > 22) {
                text = text.slice(0, 22);
            }
        }
    }
    Rectangle
    {
        id:recGroupName
        color: "#2f6ea5"
        width: parent.width
        height: 2
        anchors{
            left: groupName.left
            leftMargin: 6
            top: groupName.bottom
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
                createGroupForm.close()
            }
        }
    }

    Text{
        id:cancel
        text: "Cancel"
        color: "White"
        font.pointSize: 12
        anchors{
            right: create.left
            rightMargin: 10
            bottom: create.bottom
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: { createGroupForm.close() }
        }
    }

    Text{
        id:create
        text: "Create"
        color: "White"
        font.pointSize: 12
        anchors{
            right: parent.right
            rightMargin: 10
            bottom: parent.bottom
            bottomMargin: 10
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                client.createGroup(groupName.text);
                groupName.clear();
                createGroupForm.close();
            }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    onOpened: {
        overlay.opacity = 1
        createGroupForm.opacity = 1
    }

    onClosed: {
        overlay.opacity = 0
        createGroupForm.opacity = 0
    }
}
