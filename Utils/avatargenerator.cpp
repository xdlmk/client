#include "avatargenerator.h"

AvatarGenerator::AvatarGenerator(QObject *parent)
    : QObject{parent}
{}

void AvatarGenerator::generateAvatarImage(const QString &text, const int &id, const QString &type)
{
    QImage image(200, 200, QImage::Format_ARGB32);
    QColor baseColor = generateColor(text);

    QColor lightColor = baseColor.lighter(150);
    QLinearGradient gradient(0, 0, 0, 200);
    gradient.setColorAt(0.0, baseColor);
    gradient.setColorAt(1.0, lightColor);

    QPainter painter(&image);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, 200, 200);

    QFont font("Arial", 72, QFont::Bold);
    painter.setFont(font);

    painter.setPen(Qt::white);

    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(text.mid(0, 1));
    int textHeight = fm.height();
    int x = (200 - textWidth) / 2;
    int y = (200 + textHeight) / 2;

    painter.drawText(x, y, text.mid(0, 1));
    painter.end();

    image.save(QCoreApplication::applicationDirPath() + "/.data/" + activeUserLogin + "/avatars/" + type + "/" + QString::number(id) + ".png", "PNG");

}

void AvatarGenerator::setActiveUser(const QString &userLogin, const int &userId)
{
    this->activeUserLogin = userLogin;
    this->activeUserId = userId;
}

QColor AvatarGenerator::generateColor(const QString &text)
{
    uint hash = qHash(text);
    int r = (hash >> 16) & 0xFF;
    int g = (hash >> 8) & 0xFF;
    int b = hash & 0xFF;

    qreal red = r / 255.0;
    qreal green = g / 255.0;
    qreal blue = b / 255.0;

    qreal brightness = 0.299 * red + 0.587 * green + 0.114 * blue;

    if (brightness > 0.7) {
        red = qMax(red - 0.4, 0.0);
        green = qMax(green - 0.4, 0.0);
        blue = qMax(blue - 0.4, 0.0);
    }

    qreal luminance = 0.299 * red + 0.587 * green + 0.114 * blue;
    qreal contrast = (luminance + 0.05) / (1.0 + 0.05);

    if (contrast > 0.5) {
        red = qMax(red - 0.3, 0.0);
        green = qMax(green - 0.3, 0.0);
        blue = qMax(blue - 0.3, 0.0);
    }
    return QColor::fromRgbF(red,green,blue,1.0);
}
