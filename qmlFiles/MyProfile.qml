import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Dialog {
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0

    background: Rectangle {
            color: "#1e2a36"
            radius: 10
            border.color: "#626a72"
            border.width: 1/2
        }
    width: 300
    height: 500

    Text{
        id:myProfileText
        text: "My profile"
        color: "White"
        font.pointSize: 15
        font.bold: true

        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    /*Text{
        id:userLoginText
        text: userlogin
        color: "White"
    }*/

    Column {
        anchors.fill: parent

    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    onOpened: {
        overlay.opacity = 1
        myProfileWindow.opacity = 1
    }

    onClosed: {
        overlay.opacity = 0
        myProfileWindow.opacity = 0
    }
}
