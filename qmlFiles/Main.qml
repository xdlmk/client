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
         property string nameload: model.name
        }
         Component{
             id: outgoingDelegate
            ChatBubbleOut {
            anchors.right: parent.right
            text: textload
            time: timeload
            name: nameload
        }
         }
         Component {
             id: incomingDelegate
             ChatBubbleIn {
                anchors.left: parent.left
                text:textload
                time:timeload
                name:nameload
             }
         }
    }
    ListModel
    {
        id: listModel
        ListElement {
            text: "Wassaaa, are you here?"
            time: "no:time"
            name: "xdlmk"
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
            newMsg.name = userlogin;
            newMsg.isOutgoing = true;
            client.sendToServer(msg,newMsg.name);
        }
    }

    function onInMessage(name)
    {
        var newMsg = {};
        newMsg.text = messageFrom;
        newMsg.time = Qt.formatTime(new Date(), "hh:mm");
        newMsg.name = name;
        newMsg.isOutgoing = false;
        listModel.append(newMsg);
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }

    function onOutMessage(name)
    {
        var newMsg = {};
        newMsg.text = messageFrom;
        newMsg.time = Qt.formatTime(new Date(), "hh:mm");
        newMsg.name = name;
        newMsg.isOutgoing = true;
        listModel.append(newMsg);
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }

    Rectangle
    {
        id:connectRect
        visible:false
        anchors.left: parent.left
        anchors.top:parent.top
        anchors.right: parent.right
        color:"#9945464f"
        height: 25

        Text{
            id:textConnect
            anchors.centerIn: parent
            color:"#99FFFFFF"
            font.pointSize: 10
            text: "Connection unsuccessful, try connecting again"
        }
    }

    function connectError()
    {
        connectRect.visible = true;
    }

    function connectSuccess()
    {
        connectRect.visible = false;
    }

    Component.onCompleted:
    {
        newInMessage.connect(onInMessage);
        newOutMessage.connect(onOutMessage);
        errorWithConnect.connect(connectError);
        connectionSuccess.connect(connectSuccess)
    }


}


