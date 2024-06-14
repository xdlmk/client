import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: root
    width: 1000
    height: 500
    visible: true
    color: "#0e1621"
    title: qsTr("Hello World")
    readonly property int defMargin: 10
    readonly property color panelColor: "#17212B"
    readonly property color bgColor: "#0e1621"
    readonly property color textColor: "white"

    Page
    {
        id: page
        anchors.fill: parent


        ListView
        {
            id: listView
            anchors.fill: parent
            spacing: defMargin
            ScrollBar.vertical: ScrollBar{}

            model:ListModel
            delegate: Rectangle {
                height: 60
                anchors.left: parent.left
                anchors.right:parent.right
                anchors.margins: defMargin*2
                color: "lightgrey"
                radius: 10

                Text {
                    anchors.fill: parent
                    anchors.margins: defMargin
                    text: model.text
                }
                Text {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: defMargin/2
                    text: model.time
                }
            }
        }
        ListModel
        {
            id:listModel
            ListElement
            {
                text:"aaaa"
                time: "12:32"
            }
            ListElement
            {
                text:"bbbb"
                time:"12:33"
            }
            ListElement
            {
                text:"cccc"
                time:"12:33"
            }
            ListElement
            {
                text:"dddd"
                time:"12:34"
            }
        }
    }
}
