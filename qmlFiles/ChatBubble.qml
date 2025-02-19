import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: Math.min(lblText.implicitWidth + 20, listView.width * 0.75)
    height: lblText.implicitHeight + lblTime.implicitHeight + nameText.implicitHeight + (fileText.visible ? fileText.implicitHeight + 10 : 0) + 10

    Rectangle {
        id: rectBubble
        color: isOutgoing ? "#2b5278" : "#182533"
        radius: 10
        anchors.fill: parent

        Text{
            id: nameText
            anchors{
                left: parent.left
                leftMargin: 5
                top: parent.top
                topMargin: 3
            }
            text: name
            width: parent.width - 20
            font.pointSize: 10
            font.bold: true
            color: generateColor(name)
            wrapMode: Text.WrapAnywhere
        }

        Text {
            id: fileText
            visible: fileName !== ""
            text: fileName
            height: fileText.visible ? implicitHeight : 0
            width: fileText.visible ? implicitWidth : 0
            anchors {
                left: parent.left
                leftMargin: 5
                top: nameText.bottom
                topMargin: fileText.visible ? 5 : 0
            }
            font.pointSize: 10
            color: isOutgoing ? "white" : "#e4ecf2"
            font.bold: true
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if(lblText.text !== "") {
                        client.getFile(fileUrl);
                    } else {
                        client.getVoice(fileUrl);
                    }
                }
            }
        }

        Text{
            id: lblText
            anchors{
                left: parent.left
                leftMargin: 5
                top: fileText.visible ? fileText.bottom : nameText.bottom
                topMargin: 5
            }
            text: message
            width: parent.width - 20
            font.pointSize: 10
            color: isOutgoing ? "white" : "#e4ecf2"
            wrapMode: Text.WrapAnywhere
        }

        Text {
            id: lblTime
            anchors{
                right: parent.right
                bottom: parent.bottom
                rightMargin: 5
                bottomMargin: 5
            }
            text: time
            width: parent.width - 20
            font.pointSize: 8
            color: isOutgoing ? "#488dd3" : "#6d7f8f"
            horizontalAlignment: Text.AlignRight
        }
    }
    function generateColor(text) {
        let hash = 0;
        for (let i = 0; i < text.length; i++) {
            hash = text.charCodeAt(i) + ((hash << 5) - hash);
        }
        const r = (hash >> 16) & 0xFF;
        const g = (hash >> 8) & 0xFF;
        const b = hash & 0xFF;
        return Qt.rgba(r / 255, g / 255, b / 255, 1);
    }
}
