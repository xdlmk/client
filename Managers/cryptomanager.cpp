#include "cryptomanager.h"
#include <sodium.h>
#include <stdexcept>

CryptoManager::CryptoManager(QObject *parent)
    : QObject{parent}
{
    if (sodium_init() < 0) {
        qDebug() << "init failed!";
    }
}

CryptoKeys CryptoManager::generateKeys(const QString &password)
{
    CryptoKeys keys;

    unsigned char publicKey[crypto_box_PUBLICKEYBYTES];
    unsigned char privateKey[crypto_box_SECRETKEYBYTES];
    if (crypto_box_keypair(publicKey, privateKey) != 0) {
        throw std::runtime_error("Error generating key pait");
    }

    unsigned char salt[crypto_pwhash_SALTBYTES];
    randombytes_buf(salt, sizeof(salt));

    unsigned char symKey[crypto_secretbox_KEYBYTES];
    QByteArray passwordBytes = password.toUtf8();
    if (crypto_pwhash(symKey, crypto_secretbox_KEYBYTES,
                      passwordBytes.constData(), passwordBytes.size(),
                      salt,
                      crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        throw std::runtime_error("Error deriving symmetric key from password");
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    unsigned char encryptedPrivate[crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES];
    if (crypto_secretbox_easy(encryptedPrivate,
                              privateKey,
                              crypto_box_SECRETKEYBYTES,
                              nonce,
                              symKey) != 0) {
        throw std::runtime_error("Private Key Encryption Error");
    }

    keys.publicKey = QByteArray(reinterpret_cast<const char*>(publicKey), crypto_box_PUBLICKEYBYTES);
    keys.encryptedPrivateKey = QByteArray(reinterpret_cast<const char*>(encryptedPrivate),
                                          sizeof(encryptedPrivate));
    keys.salt = QByteArray(reinterpret_cast<const char*>(salt), crypto_pwhash_SALTBYTES);
    keys.nonce = QByteArray(reinterpret_cast<const char*>(nonce), crypto_secretbox_NONCEBYTES);

    return keys;
}
