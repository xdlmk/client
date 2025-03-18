import QtQuick

Canvas {
    id: canvas
    property string textImage: "A"
    property color baseColor: generateColor()

    width: 200
    height: 200

    onPaint: {
        var ctx = getContext("2d")
        var gradient = ctx.createLinearGradient(0, 0, 0, height)
        gradient.addColorStop(0, baseColor)
        gradient.addColorStop(1, Qt.lighter(baseColor, 1.5))
        ctx.fillStyle = gradient
        ctx.fillRect(0, 0, width, height)

        var fontSize = Math.min(width, height) * 0.5;
        ctx.font = fontSize + "px Helvetica, 'Segoe UI', Arial"
        ctx.textAlign = "center"
        ctx.textBaseline = "middle"
        ctx.fillStyle = "white"
        ctx.fillText(getFirstLetter(), width / 2, height / 2)
    }

    function generateColor() {
        let hash = 0
        for (let i = 0; i < textImage.length; i++) {
            hash = textImage.charCodeAt(i) + ((hash << 5) - hash)
        }
        let r = (hash >> 16) & 0xFF
        let g = (hash >> 8) & 0xFF
        let b = hash & 0xFF
        return Qt.rgba(r / 255, g / 255, b / 255, 1)
    }

    function getFirstLetter() {
        if (typeof textImage !== "string" || textImage.trim() === "") {
            return " ";
        }
        return textImage.trim().charAt(0).toUpperCase();
    }
}
