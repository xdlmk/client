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

        var radius = Math.min(width, height) / 2
        ctx.beginPath()
        ctx.arc(width / 2, height / 2, radius, 0, 2 * Math.PI, false)
        ctx.closePath()

        ctx.clip()

        ctx.fill()

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

        r /= 255
        g /= 255
        b /= 255

        let brightness = 0.299 * r + 0.587 * g + 0.114 * b

        if (brightness > 0.7) {
            r = Math.max(r - 0.4, 0.0)
            g = Math.max(g - 0.4, 0.0)
            b = Math.max(b - 0.4, 0.0)
        }

        let luminance = 0.299 * r + 0.587 * g + 0.114 * b
        let contrast = (luminance + 0.05) / (1.0 + 0.05)

        if (contrast > 0.5) {
            r = Math.max(r - 0.3, 0.0)
            g = Math.max(g - 0.3, 0.0)
            b = Math.max(b - 0.3, 0.0)
        }

        return Qt.rgba(r, g, b, 1)
    }

    function getFirstLetter() {
        if (typeof textImage !== "string" || textImage.trim() === "") {
            return " ";
        }
        return textImage.trim().charAt(0).toUpperCase();
    }
}
