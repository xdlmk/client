#include "avatargenerator.h"

AvatarGenerator::AvatarGenerator(QObject *parent)
    : QObject{parent}
{}

void AvatarGenerator::generateAvatarImage(const QString &text, const int &id, const QString &type)
{
    uint hash = qHash(text);
    int r = (hash >> 16) & 0xFF;
    int g = (hash >> 8) & 0xFF;
    int b = hash & 0xFF;

    QImage image(200, 200, QImage::Format_ARGB32);
    QColor baseColor = QColor(r, g, b);

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
