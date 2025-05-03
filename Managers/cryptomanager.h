#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QByteArray>
#include <QApplication>

struct CryptoKeys {
    QByteArray publicKey;
    QByteArray encryptedPrivateKey;
    QByteArray salt;
    QByteArray nonce;
};

class CryptoManager : public QObject
{
    Q_OBJECT
public:
    explicit CryptoManager(QObject *parent = nullptr);

    void setLastEnteredPassword(const QString &password);

    CryptoKeys generateKeys(const QString &password);
    bool decryptAndSavePrivateKey(const QByteArray &encryptedPrivateKey, const QByteArray &salt, const QByteArray &nonce, const QString &filePath);

signals:

private:
    QString lastEnteredPassword;
};

#endif // CRYPTOMANAGER_H
