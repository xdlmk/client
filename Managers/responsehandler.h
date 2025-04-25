#ifndef RESPONSEHANDLER_H
#define RESPONSEHANDLER_H

#include <QObject>

#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "Utils/logger.h"

#include "generated_protobuf/login.qpb.h"
#include "generated_protobuf/register.qpb.h"
#include "generated_protobuf/search.qpb.h"
#include "generated_protobuf/editProfile.qpb.h"
#include "generated_protobuf/avatarsUpdate.qpb.h"
#include "generated_protobuf/deleteMember.qpb.h"
#include "generated_protobuf/addMembers.qpb.h"
#include "generated_protobuf/chatsInfo.qpb.h"
#include "generated_protobuf/identifiers.qpb.h"
#include <QtProtobuf/qprotobufserializer.h>

class ResponseHandler : public QObject
{
    Q_OBJECT
public:
    explicit ResponseHandler(QObject *parent = nullptr);
    void setLogger(Logger *logger);
    void setActiveUser(const QString &userLogin,const int &userId);
public slots:
    void processingLoginResults(const QByteArray &loginResultsData);
    void processingRegistrationResults(const QByteArray &regResultsData);

    void processingDeleteGroupMember(const QByteArray &receivedDeleteMemberFromGroupData);
    void processingAddGroupMember(const QByteArray &receivedAddMemberFromGroupData);
    void processingDialogsInfoSave(const QList<chats::DialogInfoItem> &receivedDialogInfo);
    void processingGroupInfoSave(const QList<chats::GroupInfoItem> &receivedGroupInfo);

    void processingSearchData(const QByteArray &searchData);

    void processingEditProfile(const QByteArray &editResultsData);

    void processingAvatarsUpdate(const QByteArray &avatarsUpdateData);

signals:
    void transferUserNameAndIdAfterLogin(const QString &activeUserName,const int &activeUserId);
    void checkAndSendAvatarUpdate(const QString &avatar_url,const int &user_id,const QString& type);
    void loginSuccess(QString &name, int &user_id);
    void loginFail();
    void addAccount(const QString &login, const QString &password, int userId);
    void updatingChats();

    void sendData(const QString& flag, const QByteArray& data);

    void registrationSuccess();
    void registrationFail(QString error);

    void newSearchUser(QString &userlogin,int &id);

    void editUniqueError();
    void unknownError();
    void editUserlogin(QString editInformation);
    void editPhoneNumber(QString editInformation);
    void editName(QString editInformation);

    void getChatsInfo();

    void getGroupMembers(const int& group_id);
    void clearMessagesAfterDelete(const int& group_id);
private:
    int deleteUserFromInfoFile(const int& group_id, const int& user_id);

    bool saveProtoObjectToFile(const QString& path, const QByteArray& data);
    bool readProtoObjectFromFile(const QString &path, QByteArray &data);


    Logger *logger;
    QString activeUserLogin;
    int activeUserId;
};

#endif // RESPONSEHANDLER_H
