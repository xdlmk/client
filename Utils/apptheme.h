#ifndef APPTHEME_H
#define APPTHEME_H

#include <QObject>
#include <QColor>
#include <QSettings>

class AppTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor chatBackground READ chatBackground WRITE setChatBackground NOTIFY chatBackgroundChanged)
    Q_PROPERTY(QColor incomingColor READ incomingColor WRITE setIncomingColor NOTIFY incomingColorChanged)
    Q_PROPERTY(QColor outgoingColor READ outgoingColor WRITE setOutgoingColor NOTIFY outgoingColorChanged)

public:
    explicit AppTheme(QObject* parent = nullptr);

    QColor chatBackground() const;
    void setChatBackground(const QColor& color);

    QColor incomingColor() const;
    void setIncomingColor(const QColor& color);

    QColor outgoingColor() const;
    void setOutgoingColor(const QColor& color);

    void loadForUser(const QString &userId);
    QString configPathForUser(const QString &userId);
    Q_INVOKABLE void save();

signals:
    void chatBackgroundChanged();
    void incomingColorChanged();
    void outgoingColorChanged();

private:
    void load();

    std::unique_ptr<QSettings> m_settings;

    QColor m_chatBackground;
    QColor m_incomingColor;
    QColor m_outgoingColor;
};

#endif // APPTHEME_H
