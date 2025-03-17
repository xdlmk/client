import QtQuick

Item {
    id: root
    property alias source: image.source
    property string textImage: "A"
    property int imageWidth: 200
    property int imageHeight: 200
    property alias cache: image.cache
    property alias fillMode: image.fillMode
    property alias smooth: image.smooth

    width: imageWidth
    height: imageHeight

    Image {
        id: image
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        cache: false
        smooth: false
        fillMode: Image.PreserveAspectFit
        visible: status === Image.Ready || status === Image.Loading

        onStatusChanged: {
            if (status === Image.Error) {
                fallback.visible = true
                image.visible = false
            } else if(status === Image.Ready || status === Image.Loading) {
                fallback.visible = false
                image.visible = true
            }
        }
    }

    FallbackImage {
        id: fallback
        visible: false
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        textImage: root.textImage
    }
}
