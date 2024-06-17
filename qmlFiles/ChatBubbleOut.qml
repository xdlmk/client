import QtQuick
import QtQuick.Controls

Item {
    id: root
    property alias text: lblText.text
    property alias time: lblTime.text
    property alias name: nameText.text
    property alias backgroundColor: rectBubble.color
    property alias textColor: lblText.color
    property alias font: lblText.font
    width: Math.min(lblText.implicitWidth + 20, listView.width * 0.75)
    height: lblText.implicitHeight + lblTime.implicitHeight + nameText.implicitHeight + 5

    Rectangle {
        id: rectBubble
        color: "#2b5278"
        radius: 10
        anchors.fill: parent

        Text{
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.top: parent.top
            anchors.topMargin: 3
            id: nameText
            text: model.name
            width: parent.width - 20
            font.pointSize: 10
            font.bold: true
            color: Qt.rgba(Math.random(),Math.random(),Math.random(),255)
            wrapMode: Text.WrapAnywhere
        }
        Text{
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.top: nameText.top
            anchors.topMargin: 15
            id: lblText
            text: model.text
            width: parent.width - 20
            font.pointSize: 10
            color: "white"
            wrapMode: Text.WrapAnywhere

            padding: 0
        }

        Text {
            id: lblTime
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 5
            anchors.bottomMargin: 5
            text: model.time
            width: parent.width - 20
            font.pointSize: 8
            color: "#488dd3"
            horizontalAlignment: Text.AlignRight
        }
    }
}
