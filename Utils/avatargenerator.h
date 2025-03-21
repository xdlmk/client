#ifndef AVATARGENERATOR_H
#define AVATARGENERATOR_H

#include <QObject>

#include <QImage>
#include <QColor>
#include <QtGlobal>
#include <QFont>
#include <QPainter>
#include <QCoreApplication>

class AvatarGenerator : public QObject
{
    Q_OBJECT
public:
    explicit AvatarGenerator(QObject *parent = nullptr);
    void generateAvatarImage(const QString& text, const int& id,const QString& type);// avatar generator

    void setActiveUser(const QString &userLogin, const int &userId);
private:
    QColor generateColor(const QString& text);
    QString activeUserLogin;
    int activeUserId;
};

#endif // AVATARGENERATOR_H
