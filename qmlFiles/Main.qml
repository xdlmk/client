import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts
import QtMultimedia

Window {
    id: rootWindow
    width: 1000
    height: 500
    visible: true
    color: themeManager.chatBackground // "#0e1621"
    title: qsTr("Regagram")

    property bool isProfileExtended: false
    property bool isSearchListExtended: false
    property string avatarSource: "../../.data/" + activeUserId + "/avatars/personal/"
    property string groupAvatarSource: "../../.data/" + activeUserId + "/avatars/group/"
    property int timestamp: new Date().getTime()

    property string activeChatTypeBeforeRequest: "default"
    property int activeChatIdBeforeRequest: 0

    property var currentChatBubble: null

    Rectangle {
        id: leftLine
        color: themeManager.chatBackground
        height: rootWindow.height
        width: 54
        anchors{
            left:  parent.left
            bottom: parent.bottom
            top: parent.top
        }

        Rectangle {
            id: profile
            color: themeManager.chatBackground
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
                color: Qt.darker(themeManager.chatBackground)
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
        height: rootWindow.height
        width: rootWindow.width - (rootWindow.width / 2 + rootWindow.width / 4) - 54

        ListModel {
            id:personalChatsListModel
        }
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
            background: Rectangle { implicitWidth: 10; color: rootWindow.color }
            contentItem: Rectangle { implicitWidth: 10; color: "gray"; radius: 5 }
        }

        highlightFollowsCurrentItem: false
        focus: false
        cacheBuffer: 10000

        boundsBehavior: Flickable.StopAtBounds
        model: listModel

        delegate: ChatBubble {
            id:chatBubble
            anchors.margins: 20
            width: Math.min(rootWindow.width, listView.width * 0.45)
            property string message: model.text
            property var message_id: model.message_id
            property string time: model.time
            property string name: model.name
            property bool isOutgoing: model.isOutgoing
            property string fileUrl: model.fileUrl
            property string fileName: model.fileName
            property string special_type: model.special_type
            property real voiceDuration: model.voiceDuration
            property bool isRead: model.isRead !== undefined ? model.isRead : false

            onPlayRequested: (filePath, position) => {
                                 handlePlayRequest(chatBubble, filePath, position);
                             }
        }

        onContentYChanged: {
            let viewTop = listView.contentY;
            let viewBottom = viewTop + listView.height;
            updateUnreadCountForUser();

            for (let i = 0; i < listModel.count; i++) {
                let item = listView.itemAtIndex(i);

                if (item && !item.isRead) {
                    let itemTop = item.y;
                    let itemBottom = itemTop + item.height;

                    if (itemBottom > viewTop && itemTop < viewBottom) {
                        item.markAsRead();
                    }
                }
            }
        }

        property int savedIndexFromEnd: 0

        onAtYBeginningChanged: {
            if (atYBeginning && upLine.currentState !== "default" && listModel.count !== 0) {
                listView.savedIndexFromEnd = listModel.count;
                client.requestMessageDownload(upLine.user_id, nameText.text, upLine.currentState, listModel.count);
                activeChatIdBeforeRequest = upLine.user_id;
                activeChatTypeBeforeRequest = upLine.currentState;
            }
        }
    }

    ListModel {
        id: listModel
    }

    Rectangle {
        id: upLine
        color: Qt.lighter(themeManager.chatBackground, 1.50)
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
                    if(upLine.currentState === "personal"){
                        overlay.visible = true
                        myProfileWindow.setUserId(upLine.user_id);
                        myProfileWindow.open()
                        myProfileWindow.userProfile(nameText.text)
                    } else if(upLine.currentState === "group") {
                        overlay.visible = true
                        groupInfoForm.setGroupId(upLine.user_id);
                        groupInfoForm.setGroupName(nameText.text);
                        groupInfoForm.open();
                    }
                }
            }
        }

        Text {
            id: valueText
            text: "Offline"
            font.pointSize: 8
            color: "grey"
            visible: false
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

    GroupInfoForm {
        id: groupInfoForm
    }

    CreateGroupForm {
        id: createGroupForm
    }

    SelectContactsForm {
        id:selectContactsForm
    }

    ThemeSettings {
        id:themeSettings
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

    function onMediaPlayerDurationChanged(duration) {
        if(currentChatBubble.isActive)
            currentChatBubble.voiceDuration = duration;
    }

    function onMediaPlayerPositionChanged(position) {
        if(currentChatBubble.isActive)
            currentChatBubble.voicePosition = position;
    }

    function onMediaPlayerStateChanged(state) {
        if (state === MediaPlayer.StoppedState) {
            if (audioManager.getMediaPlayerPosition() >= audioManager.getMediaPlayerDuration()) {
                audioManager.setPosition(0);
                if (currentChatBubble.isActive) {
                    currentChatBubble.playButtonText.text = "▶";
                    currentChatBubble.voicePosition = 0;
                }
            }
        } else if (state === MediaPlayer.PlayingState) {
            currentChatBubble.playButtonText.text = "⏸";
        } else if (state === MediaPlayer.PausedState) {
            currentChatBubble.voicePosition = audioManager.getMediaPlayerPosition();
            currentChatBubble.playButtonText.text = "▶";
        }
    }

    function onVoiceExists(){
        audioManager.playVoice();
        currentChatBubble.isActive = true;
    }

    function handlePlayRequest(chatBubble, filePath, startPosition) {
        if (currentChatBubble && currentChatBubble !== chatBubble) {
            currentChatBubble.isActive = false;
        }
        audioManager.stop();

        currentChatBubble = chatBubble;
        audioManager.setSource(filePath);
        audioManager.setPosition(startPosition);

        var receiver_id;
        if(upLine.currentState === "group") receiver_id = 0;
        else if(upLine.currentState === "personal") receiver_id = upLine.user_id;
        fileManager.getFile(filePath, "voiceFileUrl", receiver_id);
    }

    function onNewMessage(data) {
        listModel.append({text: data.message, message_id: data.message_id, time: data.time,
                             name: data.login, isOutgoing: data.Out === "out" ? true : false,
                             fileName: data.fileName, fileUrl: data.fileUrl, special_type: data.special_type,
                             voiceDuration: data.audio_duration, isRead: data.isRead});
        listView.positionViewAtIndex(listModel.count - 1, ListView.End);
    }

    function addMessageToTop(data,isOutgoing) {
        if(activeChatIdBeforeRequest === upLine.user_id && activeChatTypeBeforeRequest === upLine.currentState) {
            listModel.insert(0, {text: data.message, message_id: data.message_id, time: data.time,
                                 name: data.login, isOutgoing: isOutgoing,
                                 fileName: data.fileName, fileUrl: data.fileUrl, special_type: data.special_type,
                                 voiceDuration: data.audio_duration, isRead: data.isRead});
        }
    }

    function setReadStatus(message_id, chatId, type) {
        if ((upLine.user_id === chatId) && (upLine.currentState === type)) {
            var index = -1;
            for (var i = 0; i < listModel.count; i++) {
                if (listModel.get(i).message_id === message_id) {
                    index = i;
                    break;
                }
            }

            console.log("Index message: ", index);
            if (index === -1) return;

            if (!listModel.get(index).isRead) {
                console.log("Before setProperty");
                listModel.setProperty(index, "isRead", true);

                var delegate = listView.itemAtIndex(index);
                if (delegate) {
                    console.log("Message with id: ", message_id, " setting status true");
                    delegate.isRead = true;
                }
            }
        }
    }

    function connectError() { connectRect.visible = true; }

    function connectSuccess() { connectRect.visible = false; }

    function onCheckActiveDialog(data,type) {
        if(type === "group") {
            logger.qmlLog("INFO","Main.qml::onCheckActiveDialog","Dialog active: " + (upLine.user_id === data.group_id && upLine.currentState === type));
            if (upLine.user_id === data.group_id && upLine.currentState === type) onNewMessage(data);
        } else if (type === "personal") {
            logger.qmlLog("INFO","Main.qml::onCheckActiveDialog","Dialog active: " + ((upLine.user_id === data.id || upLine.user_id === data.second_id) && upLine.currentState === type));
            if ((upLine.user_id === data.id || upLine.user_id === data.second_id) && upLine.currentState === type) onNewMessage(data);
        }
    }

    function updateUnreadCountForUser() {
        var targetIndex = -1;
        for (var i = 0; i < personalChatsListModel.count; i++) {
            if ((personalChatsListModel.get(i).id === upLine.user_id) && (personalChatsListModel.get(i).currentChatType === upLine.currentState)) {
                targetIndex = i;
                break;
            }
        }
        if (targetIndex !== -1) {
            var delegateItem = centerLine.chatsListView.itemAtIndex(targetIndex);
            if (delegateItem !== null) {
                delegateItem.countUnreadMessages();
            }
        }
    }

    function loadCountOfGroupMembers(jsonArray, group_id){
        valueText.text = jsonArray.length + " participants";
        valueText.visible = true;
    }

    function returnPosition() {
        listView.forceLayout()
        listView.positionViewAtIndex(listModel.count - listView.savedIndexFromEnd, ListView.Beginning)
    }

    function onClearMainListView() { listModel.clear(); }
    function onClearMessagesAfterDelete(group_id) {
        if(upLine.currentState === "group" && upLine.user_id === group_id) {
            listModel.clear();
            upLine.currentState = "default";
        }
    }

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
        voiceExists.connect(onVoiceExists);
        clearMainListView.connect(onClearMainListView);
        clearMessagesAfterDelete.connect(onClearMessagesAfterDelete);
        loadGroupMembers.connect(loadCountOfGroupMembers);

        newMessage.connect(onNewMessage);
        insertMessage.connect(addMessageToTop);
        setReadStatusToMessage.connect(setReadStatus);
        checkActiveDialog.connect(onCheckActiveDialog);
        returnChatToPosition.connect(returnPosition);

        connectionError.connect(connectError);
        connectionSuccess.connect(connectSuccess);

        audioManager.durationChanged.connect(onMediaPlayerDurationChanged);
        audioManager.positionChanged.connect(onMediaPlayerPositionChanged);
        audioManager.stateChanged.connect(onMediaPlayerStateChanged);
    }
}
