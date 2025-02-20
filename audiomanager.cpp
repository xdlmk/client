#include "audiomanager.h"

AudioManager::AudioManager(QObject *parent)
    : QObject(parent), captureSession(new QMediaCaptureSession(this)),
    recorder(new QMediaRecorder(this)), audioInput(new QAudioInput(this)),
    player(new QMediaPlayer(this)), audioOutput(new QAudioOutput(this)) {
    player->setAudioOutput(audioOutput);
}

void AudioManager::startRecording() {
    captureSession->setAudioInput(audioInput);
    captureSession->setRecorder(recorder);
    recorder->setQuality(QMediaRecorder::HighQuality);
    recorder->setMediaFormat(QMediaFormat(QMediaFormat::Wave));
    QDir dir(QCoreApplication::applicationDirPath() + "/.tempData/" + activeUserLogin + "/voice_messages");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    qDebug() << dir;
    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath()) + "/.tempData/" + activeUserLogin + "/voice_messages" + "/voiceMessage.wav";
    qDebug() << filePath;
    recorder->setOutputLocation(QUrl::fromLocalFile(filePath));
    recorder->record();
}

void AudioManager::stopRecording() {
    recorder->stop();
}

void AudioManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserLogin = userName;
    activeUserId = userId;
}

void AudioManager::setLogger(Logger *logger)
{
    this->logger = logger;
}
