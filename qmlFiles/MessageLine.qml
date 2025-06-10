import QtQuick 2.15
import QtQuick.Controls 2.15
import QtCore
import QtQuick.Layouts

Rectangle {
    id: messageLineRoot
    readonly property int defMargin: 10
    property int maxHeight: 150
    color: adjustColor(themeManager.chatBackground, 1.50, false)
    height: Math.max(Math.min(edtText.implicitHeight,maxHeight),54) + file.height + (file.visible ? 10 : 0)
    width: parent.width/2 + parent.width/4

    property alias edtText: edtText
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
        color: themeManager.outgoingColor
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
            color: adjustColor(themeManager.outgoingColor, 1, true)
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
                color: isColorLight(fileIcon.color) ? "black" : "white"
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
            color: isColorLight(file.color) ? "black" : "white"
            elide: Text.ElideRight
        }
        Text{
            id:closeButton
            text: "✕"
            color: isColorLight(file.color) ? "black" : "white"
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

    Item {
        id: sendMessageContainer
        anchors{
            top:file.visible ? file.bottom : parent.top
            topMargin: 10
            left:parent.left
            right: parent.right
            bottom: parent.bottom
        }

        Rectangle {
            id: btnFileItem
            width: 50
            height: 50
            color: downLine.color
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }

            Button {
                id: buttonFileImage
                anchors.centerIn: parent
                background: Item { }
                icon.cache: false
                icon.source: "../images/attach.svg"
                icon.width: parent.width/2
                icon.height: parent.height/2
                icon.color: themeManager.outgoingColor
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

        ScrollView {
            id:textScrollView
            anchors {
                left: btnFileItem.right
                right: emojiButton.left
                verticalCenter: parent.verticalCenter
                top: parent.top
                bottom: parent.bottom
            }
            clip:true

            TextArea {
                id: edtText
                selectByMouse: true
                width: parent.width
                placeholderText: "Write message..."
                placeholderTextColor: isColorLight(messageLineRoot.color) ? "darkgrey" : "grey"
                font.pointSize: 10
                color: isColorLight(messageLineRoot.color) ? "black" : "white"
                background: Rectangle {
                    color: downLine.color
                }
                wrapMode: TextEdit.Wrap

                height: Math.min(implicitHeight, maxHeight)

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        if (event.modifiers & Qt.ShiftModifier) {
                            if (edtText.text.trim() === "") {
                                event.accepted = true;
                                return;
                            }
                            let textBeforeCursor = edtText.text.slice(0, edtText.cursorPosition);
                            let textAfterCursor = edtText.text.slice(edtText.cursorPosition);
                            edtText.text = textBeforeCursor + "\n" + textAfterCursor;
                            edtText.cursorPosition = textBeforeCursor.length + 1;
                        } else {
                            wordProcessing();
                        }
                        event.accepted = true;
                    }
                }
            }
        }

        Button {
            id: emojiButton
            text: "😊"
            anchors {
                right: btnAddItem.left
                verticalCenter: parent.verticalCenter
            }

            onClicked: {
                emojiPanel.visible = !emojiPanel.visible
            }
        }

        Rectangle {
            id: btnAddItem
            width: 50
            height: 50
            color: downLine.color
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }

            Button {
                id: buttonImage
                anchors.centerIn: parent
                background: Item { }
                icon.cache: false
                icon.source: "../images/send1.svg"
                icon.width: parent.width/2
                icon.height: parent.height/2
                icon.color: themeManager.outgoingColor
                visible: edtText.text.trim() !== ""
            }

            Rectangle {
                id:showRecord
                color: themeManager.outgoingColor
                visible: isRecording
                anchors.centerIn: buttonVoice
                width: isRecording ? 30 : 0
                height: isRecording ? 30 : 0
                radius: isRecording ? 15 : 0
                Behavior on width { NumberAnimation{duration:500} }
                Behavior on height { NumberAnimation{duration:500} }
                Behavior on radius { NumberAnimation{duration:500} }
            }

            Button {
                id: buttonVoice
                anchors.centerIn: parent
                background: Item { }
                icon.cache: false
                icon.source: "../images/microphone.svg"
                icon.width: parent.width/2
                icon.height: parent.height/2
                icon.color: isRecording ? themeManager.incomingColor : themeManager.outgoingColor
                visible: edtText.text.trim() === ""
            }

            MouseArea {
                id: buttonMouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    if (edtText.text.trim() === "" && !isRecording) {
                        client.startRecording(upLine.user_id, upLine.currentState);
                        isRecording = !isRecording;
                    } else if (isRecording) {
                        client.stopRecording();
                        isRecording = !isRecording;
                        client.sendVoiceMessage(upLine.user_id, upLine.currentState);
                    } else wordProcessing();
                }

                onPressed: {
                    buttonImage.icon.color = themeManager.incomingColor;
                }

                onReleased: {
                    buttonImage.icon.color = themeManager.outgoingColor;
                }
            }
        }

    }

    function wordProcessing() {
        if(upLine.user_id !== 0 && upLine.currentState !== "default") {
            if (edtText.text.trim() !== "") {
                if(fileLoad) {
                    client.sendMessageWithFile(edtText.text.trim(), upLine.user_id, filePath, upLine.currentState)
                    fileLoad = false;
                    filePath = "";
                } else {
                    client.sendMessage(edtText.text.trim(), upLine.user_id, upLine.currentState);
                }
                edtText.clear();
            }
        }
    }

    function clearData() {
        fileLoad = false;
        filePath = "";
        edtText.clear();
    }
}
