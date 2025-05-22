#include "AppTheme.h"
#include <QStandardPaths>
#include <QDir>

AppTheme::AppTheme(QObject* parent)
    : QObject(parent) {

    m_settings = std::make_unique<QSettings>
        (QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
             + "/themeConfigs/defaultColors.ini", QSettings::IniFormat);
    load();
}

void AppTheme::load() {
    m_chatBackground = m_settings->value("chatBackground", QColor("#0e1621")).value<QColor>();
    m_incomingColor = m_settings->value("incomingColor", QColor("#182533")).value<QColor>();
    m_outgoingColor = m_settings->value("outgoingColor", QColor("#2b5278")).value<QColor>();

    emit chatBackgroundChanged();
    emit incomingColorChanged();
    emit outgoingColorChanged();
}

void AppTheme::save() {
    m_settings->setValue("chatBackground", m_chatBackground);
    m_settings->setValue("incomingColor", m_incomingColor);
    m_settings->setValue("outgoingColor", m_outgoingColor);
}

QColor AppTheme::chatBackground() const { return m_chatBackground; }
void AppTheme::setChatBackground(const QColor& color) {
    if (m_chatBackground != color) {
        m_chatBackground = color;
        save();
        emit chatBackgroundChanged();
    }
}

QColor AppTheme::incomingColor() const { return m_incomingColor; }
void AppTheme::setIncomingColor(const QColor& color) {
    if (m_incomingColor != color) {
        m_incomingColor = color;
        save();
        emit incomingColorChanged();
    }
}

QColor AppTheme::outgoingColor() const { return m_outgoingColor; }
void AppTheme::setOutgoingColor(const QColor& color) {
    if (m_outgoingColor != color) {
        m_outgoingColor = color;
        save();
        emit outgoingColorChanged();
    }
}

void AppTheme::loadForUser(const QString &userId)
{
    m_settings = std::make_unique<QSettings>(configPathForUser(userId), QSettings::IniFormat);
    load();
}

QString AppTheme::configPathForUser(const QString &userId)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
           + "/themeConfigs/" + userId + "/colors.ini";
}
