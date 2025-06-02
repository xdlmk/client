import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Dialog {
    id: createGroupFormDialogRoot
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0

    background: Rectangle {
        color: adjustColor(themeManager.chatBackground, 1.5, false)
        radius: 6
    }
    width: 400
    height: 350

    property string sourcePath: ""
    property string group_name: groupName.text

    Text{
        id:createGroupText
        text: "Create Group"
        color: isColorLight(createGroupFormDialogRoot.background.color) ? "black" : "white"
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
        placeholderTextColor: isColorLight(createGroupFormDialogRoot.background.color) ? "darkgrey" : "grey"
        anchors.top: createGroupText.bottom
        anchors.topMargin: 10 + height
        font.pointSize: 10
        color: isColorLight(createGroupFormDialogRoot.background.color) ? "black" : "white"
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
        color: adjustColor(themeManager.chatBackground, 1.8, false)
        width: parent.width
        height: 2
        anchors{
            left: groupName.left
            leftMargin: 6
            top: groupName.bottom
        }
    }

    TextField{
        id:avatarPath
        width:parent.width
        selectByMouse: false
        placeholderText: "Select image"
        placeholderTextColor: isColorLight(createGroupFormDialogRoot.background.color) ? "darkgrey" : "grey"
        text: sourcePath
        anchors{
            left: groupName.left
            top: recGroupName.bottom
            topMargin: 30
        }
        font.pointSize: 10
        color: isColorLight(createGroupFormDialogRoot.background.color) ? "black" : "white"
        background: Rectangle { color: "transparent" }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                sourcePath = fileManager.openFile("Image");
            }
        }
    }
    Rectangle
    {
        id:recavatarPath
        color: adjustColor(themeManager.chatBackground, 1.8, false)
        width: parent.width
        height: 2
        anchors{
            left: avatarPath.left
            leftMargin: 6
            top: avatarPath.bottom
        }
    }

    Text{
        id:closeButton
        text: "✕"
        color: isColorLight(createGroupFormDialogRoot.background.color) ? "black" : "white"
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
        color: isColorLight(createGroupFormDialogRoot.background.color) ? "black" : "white"
        font.pointSize: 12
        anchors{
            right: nextButton.left
            rightMargin: 10
            bottom: nextButton.bottom
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: { createGroupForm.close() }
        }
    }

    Text{
        id:nextButton
        text: "Next"
        color: isColorLight(createGroupFormDialogRoot.background.color) ? "black" : "white"
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
                selectContactsForm.setParams("create");
                selectContactsForm.open();
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
        sourcePath = ""
        groupName.text = ""
        overlay.opacity = 0
        createGroupForm.opacity = 0
    }
}
