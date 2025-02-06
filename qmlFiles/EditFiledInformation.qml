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
        id:errorMessage
        text: "This login is already taken"
        font.pointSize: 10
        anchors.top: recEditField.bottom
        anchors.topMargin: 10
        anchors.left: recEditField.left

        color: "#2f6ea5"
        opacity: 0

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    Text{
        id:cancel
        text: "Cancel"
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
        anchors.rightMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        MouseArea {
            id: saveButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                client.sendEditProfileRequest(editableFiled.placeholderText,editableFiled.text)
                //editInformation.close()
                //myProfileEdit.close()
                //myProfileWindow.close()
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
        recEditField.color = "#2f6ea5"
        errorMessage.color = "#2f6ea5"
        errorMessage.opacity = 0
    }

    Timer {
        id: resetColorTimer
        interval: 5000
        repeat: false
        onTriggered: {
            recEditField.color = "#2f6ea5"
            errorMessage.color = "#2f6ea5"
        }
    }

    function editSuccess(string){
        editInformation.close()
    }

    function editError(){
        editableFiled.clear()
        errorMessage.opacity = 1
        errorMessage.color = "#ef5959"
        recEditField.color = "#ef5959"
        resetColorTimer.start()
    }

    function unknownError(){
        errorMessage.text = "Unknown error, please try again later"
        editError();
    }

    Component.onCompleted: {
        editUniqueError.connect(editError);
        editName.connect(editSuccess);
        editPhoneNumber.connect(editSuccess);
        editUserlogin.connect(editSuccess);
        unknownError.connect(unknownError);
    }
}
