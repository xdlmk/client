import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts


Window {
    id: root
    width: 1000
    height: 500
    visible: true
    color: "#0e1621"
    title: qsTr("Blockgram")

    Rectangle
    {
        id: leftLine
        color: "#0e1621"
        border.color: "black"
        border.width: 0.5
        height: root.height
        width: 54
        anchors.left:  parent.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top
    }

    Rectangle
    {
        id: centerLine
        color: "#17212b"
        border.color: "black"
        border.width: 0.5
        height: root.height
        width: root.width - (root.width/2 + root.width/4) - 54
        anchors.left:  parent.left
        anchors.leftMargin: 54
        anchors.bottom: parent.bottom
        anchors.top: parent.top
    }


    ListView {
        id: listView
        anchors.topMargin: 5
        anchors.top : upLine.bottom
        anchors.left: centerLine.right
        anchors.leftMargin: 5
        anchors.right:parent.right
        anchors.bottom: downLine.top
        spacing: 5
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            background: Rectangle {
                implicitWidth: 10
                color: root.color
            }
            contentItem: Rectangle {
                implicitWidth: 10
                color: "gray"
                radius: 5
            }
            }
        model: listModel

        delegate: Loader
        {
         anchors.margins: 10*2
         width: Math.min(root.width, listView.width * 0.45)
         active:true
         sourceComponent: model.isOutgoing ? outgoingDelegate : incomingDelegate
         property string textload: model.text
         property string timeload: model.time
        }
         Component{
             id: outgoingDelegate
            ChatBubbleOut {
            anchors.right: parent.right
            text: textload
            time: timeload
        }
         }
         Component {
             id: incomingDelegate
             ChatBubbleIn {
                anchors.left: parent.left
                text:textload
                time:timeload
             }
         }
    }
    ListModel
    {
        id: listModel
        ListElement {
            text: "Wassaaa, are you here?"
            time: "no:time"
            isOutgoing: false
        }
    }

    Rectangle
    {
        id: upLine
        color: "#17212b"
        border.color: "black"
        border.width: 0.5
        height: 60
        anchors.left:  centerLine.right
        anchors.top: parent.top
        anchors.right: parent.right

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            id: nameText
            text: "Chat"
            font.pointSize: 10
            color: "white"
        }
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            id: valueText
            text: "5 participants"
            font.pointSize: 8
            color: "grey"
        }
    }

    MessageLine {
        id: downLine
        border.color: "black"
        border.width: 0.5
        onNewMessage: {
            var newMsg = {};
            newMsg.text = msg;
            newMsg.time = Qt.formatTime(new Date(), "hh:mm");
            newMsg.isOutgoing = true;
            listModel.append(newMsg);
            client.sendToServer(msg);
            listView.positionViewAtIndex(listModel.count - 1, ListView.End);
        }
    }

    function onInMessage()
    {
        var newMsg = {};
        newMsg.text = messageFrom;
        newMsg.time = Qt.formatTime(new Date(), "hh:mm");
        newMsg.isOutgoing = false;
        listModel.append(newMsg);
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }
    Component.onCompleted:
    {
        newInMessage.connect(onInMessage);
    }

}


