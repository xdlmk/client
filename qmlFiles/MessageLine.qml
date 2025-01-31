import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Rectangle{
    readonly property int defMargin: 10
    property int maxHeight: 150
    id: downLine
    color: "#17212b"
    height: Math.max(Math.min(edtText.implicitHeight,maxHeight),54)
    width: parent.width/2 + parent.width/4

    property alias textColor: edtText.color
    signal newMessage(string msg)

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

                ScrollBar.horizontal: ScrollBar {
                    policy: Qt.ScrollBarAlwaysOff
                }

                height: Math.min(implicitHeight, maxHeight)

                Keys.onPressed: {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        if (event.modifiers & Qt.ShiftModifier) {
                        } else {

                            if (edtText.text.trim() !== "") {
                                if(upLine.currentState == "default"){
                                    //newMessage(edtText.text);
                                }
                                else if (upLine.currentState == "personal") {
                                    var newMsg = {};
                                    newMsg.text = edtText.text;
                                    newMsg.time = Qt.formatTime(new Date(), "hh:mm");
                                    newMsg.name = userlogin;
                                    newMsg.isOutgoing = true;
                                    console.log(nameText.text);
                                    client.sendPersonalMessage(edtText.text, nameText.text,upLine.user_id);
                                }

                                edtText.clear();
                            }
                        }
                        event.accepted = true;
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
                    if (edtText.text.trim() !== "") {
                        if(upLine.currentState == "default"){
                            //newMessage(edtText.text);
                        }
                        else if (upLine.currentState == "personal") {
                            var newMsg = {};
                            newMsg.text = edtText.text;
                            newMsg.time = Qt.formatTime(new Date(), "hh:mm");
                            newMsg.name = userlogin;
                            newMsg.isOutgoing = true;
                            console.log("upLine.userid: " + upLine.user_id);
                            client.sendPersonalMessage(edtText.text, nameText.text,upLine.user_id);
                        }

                        edtText.clear();
                    }
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
}
