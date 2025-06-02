import QtQuick
import QtQuick.Controls

Rectangle {
    id: rectBubble
    width: 200
    height: lblText.implicitHeight + lblTime.implicitHeight + nameText.implicitHeight + 10
    color: isOutgoing ? outgoingColor : incomingColor
    radius: 10

    property bool isOutgoing: false
    property bool isRead: false
    property string userName: ""
    property string message: ""
    property string time: ""


    Text {
        id: nameText
        anchors {
            left: parent.left
            leftMargin: 5
            top: parent.top
            topMargin: 3
        }
        text: userName
        width: parent.width - 20
        font.pointSize: 10
        font.bold: true
        color: generateColor(userName)
        wrapMode: Text.WrapAnywhere
    }

    Text{
        id: lblText
        anchors{
            left: parent.left
            leftMargin: 5
            top: nameText.bottom
            topMargin: 5
        }
        text: message
        width: parent.width - 20
        font.pointSize: 10
        color: isColorLight(rectBubble.color) ? "black" : "white"
        wrapMode: Text.WrapAnywhere
    }

    Text {
        id: lblTime
        anchors{
            right: isOutgoing ? lblReadStatus.left : parent.right
            bottom: parent.bottom
            rightMargin: 5
            bottomMargin: 5
        }
        text: time
        width: parent.width - 20
        font.pointSize: 8
        color: isOutgoing ? adjustColor(outgoingColor, 1.5, false) : adjustColor(incomingColor, 7, false)
        horizontalAlignment: Text.AlignRight
    }

    Text {
        id: lblReadStatus
        visible: isOutgoing
        anchors{
            right: parent.right
            bottom: parent.bottom
            rightMargin: 5
            bottomMargin: 5
        }
        text: isRead ? "✓✓" : "✓"
        font.pointSize: 8
        color: isRead ? adjustColor(outgoingColor, 1.5, false) : adjustColor(incomingColor, 7, false)
        horizontalAlignment: Text.AlignRight
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
