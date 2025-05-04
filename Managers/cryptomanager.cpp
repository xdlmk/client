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

void CryptoManager::setLastEnteredPassword(const QString &password)
{
    this->lastEnteredPassword = password;
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

    this->lastEnteredPassword = password;

    return keys;
}

bool CryptoManager::decryptAndSavePrivateKey(const QByteArray &encryptedPrivateKey, const QByteArray &salt, const QByteArray &nonce, const QString &filePath)
{
    if (salt.size() != crypto_pwhash_SALTBYTES) {
        throw std::runtime_error("Incorrect salt size");
    }
    if (nonce.size() != crypto_secretbox_NONCEBYTES) {
        throw std::runtime_error("Incorrect nonce size");
    }
    if (encryptedPrivateKey.size() != crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES) {
        throw std::runtime_error("Invalid encrypted private key size");
    }

    unsigned char symKey[crypto_secretbox_KEYBYTES];
    QByteArray passwordBytes = lastEnteredPassword.toUtf8();
    if (crypto_pwhash(symKey, crypto_secretbox_KEYBYTES,
                      passwordBytes.constData(), passwordBytes.size(),
                      reinterpret_cast<const unsigned char*>(salt.constData()),
                      crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0)
    {
        throw std::runtime_error("Symmetric key output error");
    }

    QByteArray decryptedKey;
    decryptedKey.resize(crypto_box_SECRETKEYBYTES);
    if (crypto_secretbox_open_easy(
            reinterpret_cast<unsigned char*>(decryptedKey.data()),
            reinterpret_cast<const unsigned char*>(encryptedPrivateKey.constData()),
            encryptedPrivateKey.size(),
            reinterpret_cast<const unsigned char*>(nonce.constData()),
            symKey) != 0)
    {
        throw std::runtime_error("Error decrypting private key");
    }

    QFile file(filePath);

    QDir dir = QFileInfo(file).absoluteDir();
    if (!dir.exists()) dir.mkpath(".");

    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Failed to open file to write private key");
    }
    qint64 bytesWritten = file.write(decryptedKey);
    file.close();
    if (bytesWritten != decryptedKey.size()) {
        throw std::runtime_error("Failed to save private key");
    }
    return true;
}

QByteArray CryptoManager::sealData(const QByteArray &data, const QByteArray &publicKey)
{
    if (publicKey.size() != crypto_box_PUBLICKEYBYTES) {
        throw std::runtime_error("Invalid public key size");
    }

    size_t sealedLen = static_cast<size_t>(data.size()) + crypto_box_SEALBYTES;
    QByteArray sealed;
    sealed.resize(sealedLen);

    if (crypto_box_seal(
            reinterpret_cast<unsigned char*>(sealed.data()),
            reinterpret_cast<const unsigned char*>(data.constData()),
            data.size(),
            reinterpret_cast<const unsigned char*>(publicKey.constData()))
        != 0) {
        throw std::runtime_error("Error encrypting data (crypto_box_seal)");
    }

    return sealed;
}

QByteArray CryptoManager::unsealData(const QByteArray &sealedData)
{
    QByteArray publicKey = loadPublicKey();
    QByteArray secretKey = loadPrivateKey();

    if (sealedData.size() < crypto_box_SEALBYTES) {
        throw std::runtime_error("Invalid sealed data size");
    }

    size_t messageLen = sealedData.size() - crypto_box_SEALBYTES;
    QByteArray decryptedData;
    decryptedData.resize(messageLen);

    if (crypto_box_seal_open(
            reinterpret_cast<unsigned char*>(decryptedData.data()),
            reinterpret_cast<const unsigned char*>(sealedData.constData()),
            sealedData.size(),
            reinterpret_cast<const unsigned char*>(publicKey.constData()),
            reinterpret_cast<const unsigned char*>(secretKey.constData()))
        != 0) {
        throw std::runtime_error("Decrypt data error (crypto_box_seal_open)");
    }

    return decryptedData;
}

QByteArray CryptoManager::symmetricEncrypt(const QByteArray &plainText, const QByteArray &sessionKey)
{
    if (sessionKey.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid session key size");
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, crypto_secretbox_NONCEBYTES);

    size_t cipherTextLen = plainText.size() + crypto_secretbox_MACBYTES;
    QByteArray cipherText;
    cipherText.resize(cipherTextLen);

    if (crypto_secretbox_easy(
            reinterpret_cast<unsigned char*>(cipherText.data()),
            reinterpret_cast<const unsigned char*>(plainText.constData()),
            plainText.size(),
            nonce,
            reinterpret_cast<const unsigned char*>(sessionKey.constData())
            ) != 0) {
        throw std::runtime_error("Error symmetric encrypting data (crypto_secretbox_easy)");
    }

    QByteArray encrypted;
    encrypted.append(reinterpret_cast<const char*>(nonce), crypto_secretbox_NONCEBYTES);
    encrypted.append(cipherText);

    return encrypted;
}

QByteArray CryptoManager::symmetricDecrypt(const QByteArray &encryptedData, const QByteArray &sessionKey)
{
    if (sessionKey.size() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid session key size");
    }

    if (encryptedData.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
        throw std::runtime_error("Encrypted data is too short");
    }

    QByteArray nonce = encryptedData.left(crypto_secretbox_NONCEBYTES);
    QByteArray cipherText = encryptedData.mid(crypto_secretbox_NONCEBYTES);

    size_t plainTextLen = cipherText.size() - crypto_secretbox_MACBYTES;
    QByteArray plainText;
    plainText.resize(plainTextLen);

    if (crypto_secretbox_open_easy(
            reinterpret_cast<unsigned char*>(plainText.data()),
            reinterpret_cast<const unsigned char*>(cipherText.constData()),
            cipherText.size(),
            reinterpret_cast<const unsigned char*>(nonce.constData()),
            reinterpret_cast<const unsigned char*>(sessionKey.constData())) != 0) {
        throw std::runtime_error("Error in symmetric decryption (crypto_secretbox_open_easy)");
    }

    return plainText;
}

QByteArray CryptoManager::loadPrivateKey()
{
    QString privateKeyPath = QCoreApplication::applicationDirPath() + "/.data/crypto/private_key.pem";

    QFile file(privateKeyPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QByteArray privateKey = file.readAll();
    file.close();

    return privateKey;
}

QByteArray CryptoManager::loadPublicKey()
{
    QString publicKeyPath = QCoreApplication::applicationDirPath() + "/.data/crypto/public_key.pem";

    QFile file(publicKeyPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QByteArray publicKey = file.readAll();
    file.close();

    return publicKey;
}
