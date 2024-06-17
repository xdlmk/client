import QtQuick 2.15

Window {
    id: root
    width: 1000
    height: 500
    visible: true
    title: qsTr("Blockgram")

    Loader {
        id: pageLoader
        anchors.fill: parent
        source: "LoginPage.qml"
    }
}
