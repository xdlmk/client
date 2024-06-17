import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts


Rectangle {

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
            anchors.margins: defMargin

            TextField {
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
            }

            Button {
                id: btnAddItem
                width: 55
                Layout.alignment: Qt.AlignVCenter
                text: ">"

                background: Rectangle
                {
                    color:downLine.color
                }

                onClicked: {
                    if (edtText.text.trim() !== "") {
                        newMessage(edtText.text);
                        edtText.clear();
                    }
                }
            }
        }
    }
