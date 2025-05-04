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

    QByteArray sealData(const QByteArray &data, const QByteArray &publicKey);
    QByteArray unsealData(const QByteArray &sealedData);

    QByteArray symmetricEncrypt(const QByteArray &plainText, const QByteArray &sessionKey);
    QByteArray symmetricDecrypt(const QByteArray &encryptedData, const QByteArray &sessionKey);

private:
    QByteArray loadPrivateKey();
    QByteArray loadPublicKey();
    QString lastEnteredPassword;
};

#endif // CRYPTOMANAGER_H
