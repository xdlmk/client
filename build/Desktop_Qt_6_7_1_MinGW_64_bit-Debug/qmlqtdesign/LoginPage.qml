import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts


Window {
    id: root
    width: 1000
    height: 500
    visible: true
    color: "#17212b"
    title: qsTr("Blockgram")

    Text{
        anchors.bottom: columnText.top
        anchors.right:columnText.right
        anchors.rightMargin: confirmButton.width/2 - 25
        text: "Login"
        color: "white"
        font.pointSize: 15
    }

    Rectangle
    {
        id: confirmButton
        width: 250
        height: 50
        radius: 10
        color:"#2f6ea5"
        anchors.centerIn: parent
        Text {
            anchors.centerIn: parent
            text: "Confirm"
            color: "white"
            font.pointSize: 15
        }
        MouseArea
        {
            id:mouseArea
            anchors.fill: parent

            onClicked: {
                client.login(loginField.text,passwordField.text);
                loginField.clear();
                passwordField.clear();
            }
            onHoveredChanged: {
                if (mouseArea.containsMouse) {
                    confirmButton.color = "#4884b3";
                } else {
                    confirmButton.color = "#2f6ea5";
                }
            }
        }
    }
    ColumnLayout
    {
        id:columnText
        anchors.bottom: confirmButton.top
        anchors.bottomMargin: 20
        anchors.left: confirmButton.left
        anchors.leftMargin: -6
        anchors.right:confirmButton.right
        Text
        {
            id:textLog
            text:"Login"
            font.pointSize: 10
            font.bold: true
            color: "#2f6ea5"
            anchors.bottom: loginField.top
            anchors.left: loginField.left
            anchors.leftMargin: 6
        }

        TextField
        {
            id:loginField
            selectByMouse: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            font.pointSize: 10
            color: "white"
            background: Rectangle {
                color: root.color
            }
        }
        Rectangle
        {
            id:recLog
            color: "#2f6ea5"
            width: 250
            height: 2
            anchors.left: loginField.left
            anchors.leftMargin: 6
        }

        Text
        {
            id:textPass
            text:"Password"
            font.pointSize: 10
            font.bold: true
            color: "#2f6ea5"
            anchors.bottom: passwordField.top
            anchors.left: passwordField.left
            anchors.leftMargin: 6
        }
        TextField
        {
            id:passwordField
            selectByMouse: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            font.pointSize: 10
            color: "white"
            echoMode: TextInput.Password
            passwordCharacter: "\u2022"
            background: Rectangle {
                color: root.color
            }
        }
        Rectangle
        {
            id:recPass
            color: "#2f6ea5"
            width: 250
            height: 2
            anchors.left: passwordField.left
            anchors.leftMargin: 6
        }
    }

    Timer {
        id: resetColorTimer
        interval: 5000
        repeat: false
        onTriggered: {
            recPass.color = "#2f6ea5";
            recLog.color = "#2f6ea5";
            textPass.color = "#2f6ea5";
            textLog.color = "#2f6ea5";
        }
    }

    function logFail()
    {
        recPass.color = "#ef5959";
        recLog.color = "#ef5959";
        textPass.color = "#ef5959";
        textLog.color = "#ef5959";
        resetColorTimer.start();

    }
    Component.onCompleted:
    {
        loginFail.connect(logFail);
    }

}
