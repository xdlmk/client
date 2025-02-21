import QtQuick 2.15
import QtQuick.Controls 2.15
import QtCore
import QtQuick.Layouts

Rectangle{
    readonly property int defMargin: 10
    property int maxHeight: 150
    color: "#17212b"
    height: Math.max(Math.min(edtText.implicitHeight,maxHeight),54) + file.height + (file.visible ? 10 : 0)
    width: parent.width/2 + parent.width/4

    property alias textColor: edtText.color
    property bool fileLoad: false
    property bool isRecording: false
    property string filePath: ""

    anchors.right:  parent.right
    anchors.bottom: parent.bottom

    visible: upLine.currentState === "default" ? false : true

    Rectangle {
        id:file
        visible: fileLoad
        enabled: visible
        height: visible ? 60 : 0
        width: visible ? 150 : 0
        color: "#2b5278"
        radius: 15
        anchors{
            top:parent.top
            topMargin: 10
            left: parent.left
            leftMargin: 10
        }
        Rectangle {
                id: fileIcon
                width: 40
                height: parent.height - 20
                color: "#1e3a5f"
                radius: 5
                anchors {
                    left: parent.left
                    leftMargin: 5
                    verticalCenter: parent.verticalCenter
                }

                Text {
                    id: extensionText
                    anchors.centerIn: parent
                    text: getExtension(filePath)
                    font.pointSize: 10
                    color: "white"
                }
            }

        Text {
            id:fileName
            text: shortenText(getFileNameFromPath(filePath), 15)
            width: parent.width - fileIcon.width - closeButton.width - 20
            anchors {
                right: parent.right
                rightMargin: 5
                verticalCenter: parent.verticalCenter
            }

            font.pointSize: 10
            color: "white"
            elide: Text.ElideRight
        }
        Text{
            id:closeButton
            text: "✕"
            color: "White"
            font.pointSize: 9
            font.bold: true
            anchors{
                right: parent.right
                rightMargin: 5
                top: parent.top
                topMargin: 5
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    filePath = "";
                    fileLoad = false;
                }
            }
        }
    }

    RowLayout {
        anchors{
            top:file.visible ? file.bottom : parent.top
            topMargin: 10
            left:parent.left
            right: parent.right
            bottom: parent.bottom
        }
        spacing: defMargin

        Layout.leftMargin: defMargin
        Layout.topMargin: defMargin

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            clip:true

            TextArea {
                id: edtText
                selectByMouse: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                placeholderText: "Write message..."
                placeholderTextColor: "grey"
                font.pointSize: 10
                color: "white"
                background: Rectangle {
                    color: downLine.color
                }
                Layout.leftMargin: defMargin
                wrapMode: TextEdit.Wrap

                height: Math.min(implicitHeight, maxHeight)

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        if (event.modifiers & Qt.ShiftModifier) {
                            edtText.text = edtText.text.slice(0, edtText.cursorPosition) + "\n" + edtText.text.slice(edtText.cursorPosition);
                            edtText.cursorPosition += 1;
                        } else {
                            wordProcessing();
                        }
                        event.accepted = true;
                    }
                }
            }
        }

        Rectangle {
            id: btnFileItem
            width: 50
            height: 50
            color: downLine.color
            Text {
                id: buttonFileImage
                text: "\u{1F4CE}"
                color: "grey"
                font.pixelSize: 24
                rotation: 45
                anchors.centerIn: parent
            }
            MouseArea {
                anchors.fill: parent
                onClicked:{
                    filePath = fileManager.openFile("All");
                    if (filePath !== "") {
                        fileLoad = true;
                        extensionText.text = getExtension(filePath);
                        fileName.text = shortenText(getFileNameFromPath(filePath),15);

                    }
                }
            }
        }

        Rectangle {
            id: btnAddItem
            width: 50
            height: 50
            color: downLine.color

            Image {
                id: buttonImage
                visible: edtText.text.trim() !== ""
                source: "../images/logo.png"
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
            }

            Rectangle {
                id:showRecord
                color: "#2b5278"
                visible: isRecording
                anchors.centerIn: buttonVoice
                width: isRecording ? 30 : 0
                height: isRecording ? 30 : 0
                radius: isRecording ? 15 : 0
                Behavior on width { NumberAnimation{duration:500} }
                Behavior on height { NumberAnimation{duration:500} }
                Behavior on radius { NumberAnimation{duration:500} }
            }
            Text {
                id: buttonVoice
                visible: edtText.text.trim() === ""
                text: "\u{1F3A4}"
                font.pointSize: 15
                anchors.centerIn: parent
            }

            MouseArea {
                id: buttonMouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    /////////
                    if (edtText.text.trim() === "" && !isRecording) {
                        client.startRecording();
                        isRecording = !isRecording;
                    } else if (isRecording) {
                        client.stopRecording();
                        isRecording = !isRecording;
                        client.sendVoiceMessage(nameText.text,upLine.user_id);
                        /////////
                    } else wordProcessing();
                }

                onPressed: {
                    buttonImage.source = "../images/logo2.png"
                }

                onReleased: {
                    buttonImage.source = "../images/logo.png"
                }
            }

        }
    }

    function wordProcessing() {
        if (edtText.text.trim() !== "") {
            if(upLine.currentState == "default"){
            }
            else if (upLine.currentState == "personal") {
                if(fileLoad) {
                    client.sendPersonalMessageWithFile(edtText.text, nameText.text,upLine.user_id,filePath)
                    fileLoad = false;
                    filePath = "";
                } else {
                    client.sendPersonalMessage(edtText.text, nameText.text,upLine.user_id);
                }
            }

            edtText.clear();
        }
    }
}
