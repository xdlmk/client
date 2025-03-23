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
    property bool rounded: true

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
        visible: false

        onStatusChanged: {
            if (status === Image.Error) {
                fallback.visible = true
                image.visible = false
                shaderEffect.visible = false
            } else if(status === Image.Ready || status === Image.Loading) {
                fallback.visible = false
                shaderEffect.visible = true
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
    ShaderEffect {
        id: shaderEffect
        anchors.fill: parent

        property Image src: image
        property real radius: root.rounded ? 1 : 0

        vertexShader: "qrc:/shaders/roundImage.vert.qsb"
        fragmentShader: "qrc:/shaders/roundImage.frag.qsb"
    }
}
