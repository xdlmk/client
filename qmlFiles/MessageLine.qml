import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Rectangle{

    readonly property int defMargin: 10
    id: downLine
    color: "#17212b"
    height: 54
    width: parent.width/2 + parent.width/4

    property alias textColor: edtText.color
    signal newMessage(string msg)

    anchors.right:  parent.right
    anchors.bottom: parent.bottom

    RowLayout {
        anchors.fill: parent
        spacing: defMargin

        Layout.leftMargin: defMargin
        Layout.topMargin: defMargin

        TextArea{
            id: edtText
            selectByMouse: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            placeholderText: "Написать сообщение..."
            placeholderTextColor: "grey"
            font.pointSize: 10
            color: "white"
            background: Rectangle {
                color: downLine.color
            }
            Layout.leftMargin: defMargin
            //wrapMode: TextEdit.Wrap

            Keys.onPressed: {
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                    if (event.modifiers & Qt.ShiftModifier) {
                        // Shift + Enter: перенос строки
                        //edtText.text += "\n";
                    } else {
                        // Enter: отправка сообщения
                        if (edtText.text.trim() !== "") {
                            newMessage(edtText.text);
                            edtText.clear();
                        }
                    }
                    event.accepted = true;
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
                        newMessage(edtText.text);
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
