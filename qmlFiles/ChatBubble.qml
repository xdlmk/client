import QtQuick
import QtQuick.Controls
import QtMultimedia

Item {
    id: root
    width: Math.min(lblText.implicitWidth + 20, listView.width * 0.75)
    height: lblText.implicitHeight + lblTime.implicitHeight + nameText.implicitHeight + (fileText.visible ? fileText.implicitHeight + 10 : 0) + 10
    property bool isActive: false
    property real voicePosition: 0
    property alias playButtonText: playButtonText

    signal playRequested(string filePath, real startPosition)

    Rectangle {
        id: rectBubble
        color: isOutgoing ? themeManager.outgoingColor : themeManager.incomingColor
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
            visible: fileName !== "" && special_type === "file_message"
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
            color: "white"
            font.bold: true
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    var receiver_id;
                    if(upLine.currentState === "group") receiver_id = 0;
                    else if(upLine.currentState === "personal") receiver_id = upLine.user_id;
                    fileManager.getFile(fileUrl,"fileUrl", receiver_id);
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
            height: lblText.visible ? implicitHeight : 0
            font.pointSize: 10
            enabled: lblText.text !== ""
            visible: lblText.text !== ""
            color: "white"
            wrapMode: Text.WrapAnywhere
        }

        Item {
            id: voiceMessage
            visible: special_type === "voice_message"
            anchors {
                top:nameText.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            Rectangle {
                anchors {
                    left:parent.left
                    leftMargin: 5
                    top:parent.top
                }
                id: playButton
                width: 30
                height: 30
                radius: 15
                color: themeManager.outgoingColor
                Text {
                    id:playButtonText
                    text: "▶"
                    font.pointSize: 15
                    anchors.centerIn: parent
                    color: "white"
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if(audioManager.getMediaPlayerPlaybackState() === MediaPlayer.StoppedState){
                            playRequested(fileUrl, voicePosition);
                        } else if (audioManager.getMediaPlayerPlaybackState() === MediaPlayer.PlayingState && isActive) {
                            audioManager.pause();
                        } else if (audioManager.getMediaPlayerPlaybackState() === MediaPlayer.PlayingState && !isActive) {
                            audioManager.pause();
                            playRequested(fileUrl, voicePosition);
                        } else if (audioManager.getMediaPlayerPlaybackState() === MediaPlayer.PausedState) {
                            playRequested(fileUrl, voicePosition);
                        }
                    }
                }

            }

            Slider {
                id: voiceSlider
                enabled: isActive
                anchors {
                    left: playButton.right
                    leftMargin: 10
                    top: playButton.top
                    topMargin: 5
                }
                width: parent.width - 100

                from: 0
                to: voiceDuration
                value: voicePosition

                onPressedChanged: {
                    if(isActive){
                        if (pressed) {
                            audioManager.pause();
                        } else {
                            audioManager.setPosition(value);
                            audioManager.playVoice();
                        }
                    }
                }

                background: Rectangle {
                    x: voiceSlider.leftPadding
                    y: voiceSlider.topPadding + voiceSlider.availableHeight / 2 - height / 2
                    implicitWidth: voiceSlider.availableWidth
                    implicitHeight: 5
                    width: voiceSlider.availableWidth
                    height: implicitHeight
                    radius: 2
                    color: Qt.lighter(themeManager.outgoingColor)

                    Rectangle {
                        width: voiceSlider.visualPosition * parent.width
                        height: parent.height
                        radius: 2
                        color: themeManager.incomingColor
                    }
                }

                handle: Rectangle {
                    x: voiceSlider.leftPadding + voiceSlider.visualPosition * (voiceSlider.availableWidth - width)
                    y: voiceSlider.topPadding + voiceSlider.availableHeight / 2 - height / 2
                    implicitWidth: 6
                    implicitHeight: 6
                    radius: 3
                    color: themeManager.incomingColor
                }
            }
            Text {
                id: lblCurrentTime
                anchors {
                    right: voiceSlider.right
                    rightMargin: 45
                    top:voiceSlider.bottom
                    topMargin: -2
                }
                text: formatTime(voicePosition)
                color: "white"
                font.pointSize: 8
            }
            Text {
                id: betweenLbls
                anchors {
                    left: lblCurrentTime.right
                    leftMargin: 5
                    top:voiceSlider.bottom
                    topMargin: -2
                }
                text: "/"
                color: "white"
                font.pointSize: 8
            }

            Text {
                id: lblDuration
                anchors {
                    left: betweenLbls.right
                    leftMargin: 5
                    top:voiceSlider.bottom
                    topMargin: -2
                }
                text: formatTime(voiceDuration)
                color: "white"
                font.pointSize: 8
            }
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
            color: isOutgoing ? Qt.lighter(themeManager.outgoingColor) : Qt.lighter(themeManager.incomingColor, 7)
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
            color: isRead ? Qt.lighter(themeManager.outgoingColor) : Qt.lighter(themeManager.incomingColor, 5)
            horizontalAlignment: Text.AlignRight
        }
    }


    function markAsRead() {
        if (!isRead && !isOutgoing) {
            isRead = true;
            client.markMessageAsRead(message_id);
        }
    }

    function formatTime(ms) {
        var totalSeconds = Math.floor(ms / 1000);
        var minutes = Math.floor(totalSeconds / 60);
        var seconds = totalSeconds % 60;
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds;
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
