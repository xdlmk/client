#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QStandardPaths>
#include <QSettings>
#include <QCoreApplication>

#include <QImage>

#include "Managers/accountmanager.h"
#include "Managers/audiomanager.h"
#include "Managers/filemanager.h"
#include "Managers/MessageManagers/messagehandler.h"
#include "Network/networkmanager.h"
#include "Utils/logger.h"

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);

    AccountManager* getAccountManager();
    FileManager *getFileManager();

signals:
    void newMessage(QVariant message);

    void connectionSuccess();
    void connectionError();

    void loginSuccess(QString &name, int &user_id);
    void loginFail();

    void registrationSuccess();
    void registrationFail(QString error);

    void clientLogout();

    void loadingChat(const QString userlogin, const QString &flag);

    void showPersonalChat(QString login,QString message, int id, QString out, QString type);
    void checkActiveDialog(QVariant message,const QString& type);

    void newSearchUser(QString userlogin,int id);

    void addAccount();

    void sendMessage(const QString &message, const int &receiver_id, const QString &flag);
    void sendMessageWithFile(const QString &message, const QString &receiver_login, const int &receiver_id,const QString& filePath, const QString &flag);
    void sendVoiceMessage(const QString &receiver_login, const int &receiver_id, const QString &flag);
    void sendSearchToServer(const QString &searchable);
    void sendLoginRequest(QString userlogin,QString password);
    void sendRegisterRequest(const QString login, const QString password);
    void sendEditProfileRequest(const QString editable,const QString editInformation);

    void editUserlogin(QString editInformation);
    void editPhoneNumber(QString editInformation);
    void editName(QString editInformation);
    void editUniqueError();
    void unknownError();

    void startRecording();
    void stopRecording();

    void clearMainListView();
    void newUser(QString username,int user_id);
    void configCheck();
    void clearUserListModel();

    void changeActiveAccount(QString username);
    void voiceExists();
    void getFile(const QString& fileUrl,const QString& flag);
    void sendNewAvatar(const QString& avatarPath, const QString &type, const int& id);
    void checkAndSendAvatarUpdate(const QString &avatar_url,const int &user_id,const QString& type);

    void createGroup(const QString& groupName, const QString& avatarPath, const QVariantList &selectedContacts);
    void addGroupMembers(const int& group_id, const QVariantList &selectedContacts);
    void getGroupMembers(const int& group_id);
    void clearMessagesAfterDelete(const int& group_id);

    void showContacts();
    void loadContacts(QVariantList contactsList);

    void loadGroupMembers(QVariantList membersList, const int& group_id);
    void deleteMemberFromGroup(const int& user_id, const int &group_id);

    void requestMessageDownload(const int &chat_id, const QString &chat_name, const QString& flag, const int& offset);
    void insertMessage(QVariant message, bool isOutgoing);
    void returnChatToPosition();

    void setLoggers(Logger* logger);
private:
    void setupConnections();
    void setupNetworkConnections();
    void setupAccountConnections();
    void setupMessageConnections();
    void setupFileConnections();
    void setupLoggingConnections();
    void setupAudioConnections();

    Logger* logger;
    NetworkManager *networkManager;
    MessageHandler *messageHandler;
    AccountManager *accountManager;
    FileManager *fileManager;
    AudioManager *audioManager;

    void setLogger(Logger* logger);
};

#endif // CLIENT_H
