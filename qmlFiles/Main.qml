import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts
import QtMultimedia

Window {
    id: rootWindow
    width: 1000
    height: 500
    visible: true
    color: themeManager.chatBackground
    title: qsTr("Regagram")

    property bool isProfileExtended: false
    property bool isSearchListExtended: false
    property string avatarSource: "../../.data/" + activeUserId + "/avatars/personal/"
    property string groupAvatarSource: "../../.data/" + activeUserId + "/avatars/group/"
    property int timestamp: new Date().getTime()

    property string activeChatTypeBeforeRequest: "default"
    property int activeChatIdBeforeRequest: 0

    property var currentChatBubble: null

    property string visibleDate: ""

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
                color: adjustColor(themeManager.chatBackground, 1.5, true)
                opacity: 0
                visible: false
                Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.OutQuad } }
            }

            Button {
                id: listImage
                anchors.centerIn: parent
                background: Item { }
                icon.cache: false
                icon.source: "../images/profile.svg"
                icon.width: parent.width/2
                icon.height: parent.height/2.5
                icon.color: adjustColor(themeManager.outgoingColor, 1.5, false)
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

    Rectangle {
        id: dateOverlay
        width: 100
        height: 20
        color: adjustColor(themeManager.chatBackground, 1.8, false)
        radius: 10
        visible: visibleDate !== ""
        anchors {
            top: listView.top
            topMargin: 10
            horizontalCenter: listView.horizontalCenter
        }
        z: 99

        Text {
            anchors.centerIn: parent
            text: visibleDate
            color: isColorLight(dateOverlay.color) ? "black" : "white"
            font.pixelSize: 12
            font.bold: true
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
            contentItem: Rectangle { implicitWidth: 10; color: "grey"; radius: 5 }
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

            let visibleDateSet = false;

            for (let i = 0; i < listModel.count; i++) {
                let item = listView.itemAtIndex(i);

                if (item) {
                    let itemTop = item.y;
                    let itemBottom = itemTop + item.height;

                    if (!visibleDateSet && itemBottom > viewTop) {
                        visibleDate = extractDateFromTimestamp(item.time);
                        visibleDateSet = true;
                    }

                    if (itemBottom > viewTop && itemTop < viewBottom && !item.isRead) {
                        item.markAsRead();
                    }
                }
            }
        }

        property int savedIndexFromEnd: 0

        onAtYBeginningChanged: {
            if (atYBeginning && upLine.currentState !== "default" && listModel.count !== 0 && !(listModel.count === 1)) {
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
        color: adjustColor(themeManager.chatBackground, 1.50, false)
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
            color: isColorLight(upLine.color) ? "black" : "white"
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
            color: isColorLight(upLine.color) ? "black" : "grey"
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
        color: "#95464f"
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

            if (index === -1) return;

            if (!listModel.get(index).isRead) {
                listModel.setProperty(index, "isRead", true);

                var delegate = listView.itemAtIndex(index);
                if (delegate) {
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

    function extractDateFromTimestamp(timestamp) {
        var date = new Date(timestamp);

        var day = date.getDate();
        var month = date.getMonth() + 1;
        var year = date.getFullYear();

        if (day < 10)
            day = "0" + day;
        if (month < 10)
            month = "0" + month;

        return day + "." + month + "." + year;
    }

    function adjustColor(colorString, factor, inverse) {
        var r = Math.round(color.r * 255);
        var g = Math.round(color.g * 255);
        var b = Math.round(color.b * 255);

        var brightness = (r * 299 + g * 587 + b * 114) / 1000;

        if (brightness < 128)
            return inverse ? Qt.darker(colorString, factor) : Qt.lighter(colorString, factor);
        else
            return inverse ? Qt.lighter(colorString, factor) : Qt.darker(colorString, factor);
    }

    function isColorLight(colorString) {
        var r = Math.round(color.r * 255);
        var g = Math.round(color.g * 255);
        var b = Math.round(color.b * 255);

        var brightness = (r * 299 + g * 587 + b * 114) / 1000;

        if (brightness < 128) return false;
        else return true;
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
