import QtQuick
import QtQuick.Controls
import QtMultimedia

Item {
    id: root
    width: Math.min(lblText.implicitWidth + 20, listView.width * 0.75)
    height: lblText.implicitHeight + lblTime.implicitHeight + nameText.implicitHeight + (fileText.visible ? fileText.implicitHeight + 10 : 0) + 10
    property bool isActive: false
    property real voicePosition: 0
    property real voiceDuration: 0

    property alias playButtonText: playButtonText

    signal playRequested(string filePath, real startPosition)

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
            color: isOutgoing ? "white" : "#e4ecf2"
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
            color: isOutgoing ? "white" : "#e4ecf2"
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
                color: "#2b5278"
                Text {
                    id:playButtonText
                    text: "▶"
                    font.pointSize: 15
                    anchors.centerIn: parent
                    color:"#ffffff"
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        playRequested(fileUrl, voicePosition);
                        /*if(globalMediaPlayer.playbackState === MediaPlayer.StoppedState){
                            playRequested(fileUrl, 0);
                        } else if (globalMediaPlayer.playbackState === MediaPlayer.PlayingState && isActive) {
                            //globalMediaPlayer.pause();
                        } else if (globalMediaPlayer.playbackState === MediaPlayer.PlayingState && !isActive) {
                            playRequested(fileUrl, 0);
                        } else if (globalMediaPlayer.playbackState === MediaPlayer.PausedState) {
                            playRequested(fileUrl, voicePosition);
                        }*/
                    }
                }

            }
            Rectangle {
                id:voiceLine
                anchors {
                    left: playButton.right
                    leftMargin: 10
                    top:playButton.top
                    topMargin: 5
                }
                width: parent.width - 100
                height: 5
                color: "#488dd3"
                radius: 2
            }
            Rectangle {
                id:voiceLineTime
                anchors {
                    left: playButton.right
                    leftMargin: 10
                    top:playButton.top
                    topMargin: 5
                }
                width: voiceLine.width * (voicePosition/voiceDuration)
                height: 5
                color: "#182533"
                radius: 2
                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeHorCursor
                    property bool isDragging: false
                    onPressed: {
                        if(isActive){
                            if (mouse.x > voiceLineTime.width - 5) {
                                isDragging = true
                                globalMediaPlayer.pause();
                            }
                        }
                    }
                    onPositionChanged: {
                        if (isDragging && isActive) {
                            let newWidth = Math.min(Math.max(mouse.x, 0), voiceLine.width)
                            voiceLineTime.width = newWidth
                            globalMediaPlayer.position = globalMediaPlayer.duration * (newWidth / voiceLine.width)
                        }
                    }

                    onReleased: {
                        isDragging = false
                    }
                }
            }
            Text {
                id: lblCurrentTime
                anchors {
                    left: playButton.right
                    leftMargin: 10
                    top:voiceLine.bottom
                    topMargin: 5
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
                    top:voiceLine.bottom
                    topMargin: 5
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
                    top:voiceLine.bottom
                    topMargin: 5
                }
                text: formatTime(voiceDuration)
                color: "white"
                font.pointSize: 8
            }
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
