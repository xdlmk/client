import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts


Item {
    Rectangle {
        id: root
        width: 1000
        height: 500
        visible: true
        color: "#17212b"

        Text{
            anchors.bottom: loginFieldsContainer.top
            anchors.right:loginFieldsContainer.right
            anchors.rightMargin: confirmButton.width/2 - 50
            text: "Registration"
            color: "white"
            font.pointSize: 15
        }

        Rectangle {
            id:regRect
            anchors.top: confirmButton.bottom
            anchors.topMargin: 15
            anchors.right: confirmButton.right
            anchors.left: confirmButton.left
            color: "#17212b"
            Text {
                id:regText
                anchors.centerIn: parent
                text: "Already have an account? Login"
                color: "#2f6ea5"
                font.pointSize: 10
                MouseArea{
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onPositionChanged: underText.visible = true
                    onExited: underText.visible = false
                    onClicked: {
                        pageLoader.source = "LoginPage.qml"
                    }
                }
            }
            Rectangle
            {
                id:underText
                visible: false
                color: "#2f6ea5"
                anchors.top: regText.bottom
                anchors.topMargin: 1
                anchors.right: regText.right
                anchors.left: regText.left
                height: 1
            }


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
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    var login = loginField.text.trim()
                    var password = passwordField.text.trim()

                    if(login === "" || password === ""){
                        recPass.color = "#ef5959";
                        recLog.color = "#ef5959";
                        textPass.color = "#ef5959";
                        textLog.color = "#ef5959";
                        resetColorTimer.start();
                    }
                    else{
                        client.sendRegisterRequest(loginField.text,passwordField.text);
                        loginField.clear();
                        passwordField.clear();
                    }
                }
                onPositionChanged: confirmButton.color = "#4884b3";
                onExited:  confirmButton.color = "#2f6ea5";
            }
        }
        Item {
            id:loginFieldsContainer
            width: loginField.width
            height: textLog.height + loginField.height + recLog.height + passwordField.height + textPass.height + recPass.height
            anchors.bottom: confirmButton.top
            anchors.bottomMargin: 36
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
                anchors.top: loginFieldsContainer.top
                anchors.topMargin: 6
                anchors.left: loginField.left
                anchors.leftMargin: 6
            }

            TextField
            {
                id:loginField
                selectByMouse: true
                anchors.top: textLog.bottom
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
                anchors.top: loginField.bottom
            }

            Text
            {
                id:textPass
                text:"Password"
                font.pointSize: 10
                font.bold: true
                color: "#2f6ea5"
                anchors.top: recLog.bottom
                anchors.topMargin: 6
                anchors.left: loginField.left
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
                anchors.top: textPass.bottom
                anchors.left: loginField.left
            }
            Rectangle
            {
                id:recPass
                color: "#2f6ea5"
                width: 250
                height: 2
                anchors.top: passwordField.bottom
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
                textLog.text = "Login";
            }
        }
        Rectangle
        {
            id:connectRect
            visible:false
            anchors.left: parent.left
            anchors.top:parent.top
            anchors.right: parent.right
            color:"#9945464f"
            height: 25

            Text{
                id:textConnect
                anchors.centerIn: parent
                color:"#99FFFFFF"
                font.pointSize: 10
                text: "Connection unsuccessful, try connecting again"
            }
        }

        function connectError()
        {
            connectRect.visible = true;
        }

        function connectSuccess()
        {
            connectRect.visible = false;
        }

        function regsFail(error)
        {
            textLog.text = error;
            recPass.color = "#ef5959";
            recLog.color = "#ef5959";
            textPass.color = "#ef5959";
            textLog.color = "#ef5959";
            resetColorTimer.start();

        }

        Component.onCompleted:
        {
            registrationFail.connect(regsFail);
            connectionError.connect(connectError);
            connectionSuccess.connect(connectSuccess)
        }

    }
}
