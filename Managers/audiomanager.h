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

#include "Core/logger.h"

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject *parent = nullptr);

    void startRecording();
    void stopRecording();

public slots:
    void setActiveUser(const QString &userName,const int &userId);
    void setLogger(Logger* logger);

signals:
    void recordingStarted();
    void recordingStopped();
    void playbackStarted();
    void playbackFinished();

private:
    QString activeUserLogin;
    int activeUserId;
    Logger* logger;

    QMediaCaptureSession *captureSession;
    QMediaRecorder *recorder;
    QAudioInput *audioInput;

    QMediaPlayer *player;
    QAudioOutput *audioOutput;
};

#endif // AUDIOMANAGER_H
