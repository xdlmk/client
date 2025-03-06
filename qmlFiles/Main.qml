import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Window {
    id: root
    width: 1000
    height: 500
    visible: true
    color: "#0e1621"
    title: qsTr("Regagram")

    property bool isProfileExtended: false
    property bool isSearchListExtended: false
    property string avatarSource: "../../avatars/" + userlogin + "/"
    property int timestamp: new Date().getTime()

    Rectangle {
        id: leftLine
        color: "#0e1621"
        height: root.height
        width: 54
        anchors{
            left:  parent.left
            bottom: parent.bottom
            top: parent.top
        }

        Rectangle {
            id: profile
            color: "#0e1621"
            height: 54
            anchors{
                left:  parent.left
                right: parent.right
                top: parent.top
            }

            Rectangle {
                id: colorOverlayProfile
                anchors.fill: parent
                anchors.margins: 1
                color: "#262d37"
                opacity: 0
                visible: false
                Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutQuad } }
            }
            Image {
                id: listImage
                source: "../images/profile.png"
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: profileMouseArea
                anchors.fill: parent
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                enabled: !isProfileExtended

                onClicked: isProfileExtended = !isProfileExtended

                onPressed: {
                    colorOverlayProfile.visible = true
                    colorOverlayProfile.opacity = 1
                }

                onReleased: {
                    colorOverlayProfile.opacity = 0
                }
            }
        }
    }

    ChatsList {
        id: centerLine
        height: root.height
        width: root.width - (root.width / 2 + root.width / 4) - 54
    }

    ListView {
        id: listView
        spacing: 5
        anchors{
            topMargin: 5
            top : upLine.bottom
            left: centerLine.right
            leftMargin: 5
            right: parent.right
            bottom: downLine.top
        }
        ScrollBar.vertical: ScrollBar {
            visible: upLine.currentState === "default" ? false : true
            background: Rectangle { implicitWidth: 10; color: root.color }
            contentItem: Rectangle { implicitWidth: 10; color: "gray"; radius: 5 }
        }

        highlightFollowsCurrentItem: false
        focus: false

        boundsBehavior: Flickable.StopAtBounds
        model: listModel

        delegate: ChatBubble {
            id:chatBubble
            anchors.margins: 20
            width: Math.min(root.width, listView.width * 0.45)
            property string message: model.text
            property string time: model.time
            property string name: model.name
            property bool isOutgoing: model.isOutgoing
            property string fileUrl: model.fileUrl
            property string fileName: model.fileName
            isWaitingForVoice: false
        }

        //property real savedDistanceFromEnd: 0.0

        onAtYBeginningChanged: {
            if (atYBeginning && upLine.currentState !== "default" && listModel.count !== 0) {
                //listView.savedDistanceFromEnd = listView.contentHeight - listView.contentY
                //console.log("Saved distance:", savedDistanceFromEnd);
                //console.log("Content height:", contentHeight);
                client.requestMessageDownload(upLine.user_id, nameText.text, upLine.currentState, listModel.count);
            }
        }
    }

    ListModel {
        id: listModel
    }

    Rectangle {
        id: upLine
        color: "#17212b"
        height: 55
        anchors{
            left:  centerLine.right
            top: parent.top
            right: parent.right
        }
        visible: currentState === "default" ? false : true
        property string currentState: "default"
        property int user_id: 0

        Text {
            id: nameText
            text: "Chat"
            font.pointSize: 10
            color: "white"
            anchors{
                left: parent.left
                leftMargin: 10
                top: parent.top
                topMargin: 10
            }
            MouseArea {
                id: openProfileMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    overlay.visible = true
                    myProfileWindow.setUserId(upLine.user_id);
                    myProfileWindow.open()
                    myProfileWindow.userProfile(nameText.text)
                }
            }
        }

        Text {
            id: valueText
            text: "5 participants"
            font.pointSize: 8
            color: "grey"
            anchors{
                left: parent.left
                leftMargin: 10
                bottom: parent.bottom
                bottomMargin: 10
            }
        }
    }

    MessageLine { id: downLine }


    Rectangle {
        id: connectRect
        visible: false
        color: "#9945464f"
        height: 25
        anchors{
            left: parent.left
            top: parent.top
            right: parent.right
        }

        Text {
            id: textConnect
            text: "Connection unsuccessful, try connecting again"
            color: "#99FFFFFF"
            anchors.centerIn: parent
            font.pointSize: 10
        }
    }

    ProfilePanel{
        id:profileWindow
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        x: isProfileExtended ? 0 : -width

        Behavior on x { NumberAnimation { duration: 500; easing.type: Easing.InOutQuad } }
    }

    MouseArea{
        id:leaveProfileArea
        enabled: isProfileExtended
        anchors{
            left: profileWindow.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        onClicked: isProfileExtended = !isProfileExtended
    }

    MouseArea{
        id:leaveSearchListArea
        anchors{
            left: centerLine.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        enabled: isSearchListExtended
        onClicked: isSearchListExtended = !isSearchListExtended
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "#80000000"
        visible: false
        opacity: 0
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
    }

    MyProfile{
        id: myProfileWindow
        user_id: 0
    }

    CreateGroupForm {
        id: createGroupForm
    }

    Timer {
        id: updateAvatarsTimer
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            timestamp = new Date().getTime();
        }
    }

    function onNewMessage(name,message,time,fileName,fileUrl,isOutgoing) {

        listModel.append({text: message, time: time, name: name, isOutgoing: isOutgoing,fileName: fileName, fileUrl: fileUrl});
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }

    function connectError() { connectRect.visible = true; }

    function connectSuccess() { connectRect.visible = false; }

    function onCheckActiveDialog(id,login,message,out,time,fileName,fileUrl,type)
    {
        logger.qmlLog("INFO","Main.qml::onCheckActiveDialog","Dialog active: " + (nameText.text === login));
        if (upLine.user_id === id && upLine.currentState === type)
        {
            if(out === "out") {
                onNewMessage(userlogin,message,time,fileName,fileUrl,true);
            } else {
                onNewMessage(login,message,time,fileName,fileUrl,false);
            }
        }
    }

    function returnPosition() {
        listView.forceLayout()
        Qt.callLater(() => {
                         //if (listView.contentHeight > 0 && listView.savedDistanceFromEnd >= 0) {
                         //console.log(listView.contentHeight, listView.savedDistanceFromEnd, listView.contentY)
                         //listView.contentY = listView.contentHeight - listView.savedDistanceFromEnd
                         //}
                     })
    }

    function addMessageToTop(name,message,time,fileName,fileUrl,isOutgoing) {
        listModel.insert(0, {text: message, time: time, name: name, isOutgoing: isOutgoing,fileName: fileName, fileUrl: fileUrl});
    }

    function onClearMainListView() { listModel.clear(); }

    function getFileNameFromPath(filePath) {
        if (!filePath || filePath.trim === "") {
            return "";
        }

        var parts = filePath.split(/[\\/]/);
        var fileName = parts[parts.length - 1];
        return fileName;
    }

    function getExtension(fullPath) {
        var fileName = getFileNameFromPath(fullPath);
        var parts = fileName.split(".");
        if (parts.length > 1) {
            return "." + parts[parts.length - 1].substring(0, 4);
        }
        return "";
    }

    function shortenText(text, maxLength) {
        if (text.length > maxLength) {
            return text.substring(0, maxLength) + "...";
        }
        return text;
    }

    Component.onCompleted: {
        clearMainListView.connect(onClearMainListView);
        newMessage.connect(onNewMessage);
        insertMessage.connect(addMessageToTop);
        checkActiveDialog.connect(onCheckActiveDialog);
        connectionError.connect(connectError);
        connectionSuccess.connect(connectSuccess);
        returnChatToPosition.connect(returnPosition);
    }
}
