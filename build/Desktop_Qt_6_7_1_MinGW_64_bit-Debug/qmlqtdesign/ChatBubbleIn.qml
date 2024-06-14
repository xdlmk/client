import QtQuick
import QtQuick.Controls

Item {

    id: root
    property alias text: lblText.text
    property alias time: lblTime.text
    property alias backgroundColor: chatBubble.color
    property alias textColor: lblText.color
    property alias font: lblText.font
    width: Math.min(lblText.implicitWidth + 20, listView.width * 0.75)
    height: lblText.implicitHeight + lblTime.implicitHeight + 5

Rectangle {
           id: chatBubble
           color: "#182533"
           radius: 10
           anchors.fill: parent

           Text {
               anchors.left: parent.left
               anchors.leftMargin: 5
               anchors.top: parent.top
               anchors.topMargin: 5
               id: lblText
               text: model.text
               width: parent.width - 20
               font.pointSize: 10
               color: "#e4ecf2"
               wrapMode: Text.WrapAnywhere

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
               color: "#6d7f8f"
               horizontalAlignment: Text.AlignRight
            }
       }
}
