#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QObject>
#include <QString>
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

    CryptoKeys generateKeys(const QString &password);

signals:
};

#endif // CRYPTOMANAGER_H
