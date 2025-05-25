import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Dialog {
    id: editInformation
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0
    background: Rectangle {
        color: Qt.lighter(themeManager.chatBackground)
        radius: 6
    }
    width: 300
    height: 200

    property string infoType: "default"

    Text{
        id:information
        text: {
            if(editInformation.infoType === "Name") return "Editing name"
            else if(editInformation.infoType === "Phone number") return "Editing phone number"
            else if(editInformation.infoType === "Username") return "Editing username"
            else if(editInformation.infoType === "default") return ""
        }
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
        id:editableFiled
        width:parent.width
        selectByMouse: true
        placeholderText: {
            if(editInformation.infoType === "Name") return "Name"
            else if(editInformation.infoType === "Phone number") return "Phone number"
            else if(editInformation.infoType === "Username") return "Username"
            else if(editInformation.infoType === "default") return ""
        }
        placeholderTextColor: "grey"
        anchors.top: information.bottom
        anchors.topMargin: 10 + height
        font.pointSize: 10
        color: "white"
        background: Rectangle { color: "transparent" }
    }
    Rectangle
    {
        id:recEditField
        color: Qt.lighter(themeManager.chatBackground, 1.8)
        width: parent.width
        height: 2
        anchors{
            left: editableFiled.left
            leftMargin: 6
            top: editableFiled.bottom
        }
    }

    Text{
        id:errorMessage
        text: "This login is already taken"
        font.pointSize: 10
        anchors{
            top: recEditField.bottom
            topMargin: 10
            left: recEditField.left
        }
        color: Qt.lighter(themeManager.chatBackground, 1.8)
        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    Text{
        id:cancel
        text: "Cancel"
        color: "White"
        font.pointSize: 12
        anchors{
            right: save.left
            rightMargin: 10
            bottom: save.bottom
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: { editInformation.close() }
        }
    }

    Text{
        id:save
        text: "Save"
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
                if (validateInput() === "ok"){
                    client.sendEditProfileRequest(editableFiled.placeholderText,editableFiled.text)
                } else {
                    errorMessage.text = validateInput();
                    editError();
                }
            }
        }
    }

    Behavior on opacity { NumberAnimation { duration: 200 } }

    onOpened: { editInformation.opacity = 1 }
    onClosed: {
        editableFiled.clear()
        editInformation.opacity = 0
        recEditField.color = Qt.lighter(themeManager.chatBackground, 1.8)
        errorMessage.color = Qt.lighter(themeManager.chatBackground, 1.8)
        errorMessage.opacity = 0
    }

    Timer {
        id: resetColorTimer
        interval: 5000
        repeat: false
        onTriggered: {
            recEditField.color = Qt.lighter(themeManager.chatBackground, 1.8)
            errorMessage.color = Qt.lighter(themeManager.chatBackground, 1.8)
        }
    }

    function validateInput() {
        if(information.text === "Username"){
            const text = editableFiled.text.trim();
            if (text.length === 0) return "The field must not be emply";
            if (text.length < 5) return "Not less than 5 characters";
            if (!/^[a-zA-Z0-9]+$/.test(text)) return "Only letters and numbers";
            if (text === myProfileEdit.login) return "You already have such a login";
        }
        if(information.text === "Phone number") return "ok";
        if(information.text === "Name") return "ok";

        return "ok";
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

    function unknownErrorFunc(){
        errorMessage.text = "Unknown error, please try again later"
        editError();
    }

    Component.onCompleted: {
        editUniqueError.connect(editError);
        editName.connect(editSuccess);
        editPhoneNumber.connect(editSuccess);
        editUserlogin.connect(editSuccess);
        unknownError.connect(unknownErrorFunc);
    }
}
