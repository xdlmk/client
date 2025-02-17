import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Rectangle{
    readonly property int defMargin: 10
    property int maxHeight: 150
    color: "#17212b"
    height: Math.max(Math.min(edtText.implicitHeight,maxHeight),54)
    width: parent.width/2 + parent.width/4

    property alias textColor: edtText.color
    property bool fileLoad: false
    property string filePath: ""

    anchors.right:  parent.right
    anchors.bottom: parent.bottom

    visible: upLine.currentState === "default" ? false : true

    RowLayout {
        anchors.fill: parent
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
                source: "../images/logo.png"
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: buttonMouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    wordProcessing();
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
