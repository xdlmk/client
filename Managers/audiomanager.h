#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QCoreApplication>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QAudioInput>
#include <QMediaPlayer>
#include <QMediaFormat>
#include <QAudioOutput>
#include <QDir>

#include "Utils/logger.h"

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject *parent = nullptr);

    void startRecording(const quint64& chat_id, const QString& type);
    void stopRecording();

public slots:
    void setActiveUser(const QString &userName,const int &userId);
    void setLogger(Logger* logger);

    void playVoice();
    void pause();
    void stop();
    void seek(qint64 position);

    QMediaPlayer::PlaybackState getMediaPlayerPlaybackState();
    qint64 getMediaPlayerDuration();
    qint64 getMediaPlayerPosition();

    void setSource(QString source);
    void setPosition(qint64 position);

signals:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void stateChanged(QMediaPlayer::PlaybackState state);

private:
    QString activeUserLogin;
    int activeUserId;
    Logger* logger;

    QMediaCaptureSession *captureSession;
    QMediaRecorder *recorder;
    QAudioInput *audioInput;

    QString recordingFilePath;
    qint64 recordingStartTime = 0;

    QMediaPlayer* player;
    QAudioOutput* audioOutput;

    qint64 position;
};

#endif // AUDIOMANAGER_H
