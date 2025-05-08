#include "audiomanager.h"

AudioManager::AudioManager(QObject *parent) : QObject(parent), captureSession(new QMediaCaptureSession(this)),
    recorder(new QMediaRecorder(this)), audioInput(new QAudioInput(this))
{ }

void AudioManager::startRecording(const quint64& chat_id, const QString& type) {
    captureSession->setAudioInput(audioInput);
    captureSession->setRecorder(recorder);
    recorder->setQuality(QMediaRecorder::HighQuality);
    recorder->setMediaFormat(QMediaFormat(QMediaFormat::Wave));
    QDir dir(QCoreApplication::applicationDirPath() + "/.tempData/" + QString::number(activeUserId) + "/voice_messages/" + type + "/" + QString::number(chat_id));
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath()) + "/.tempData/" + QString::number(activeUserId) + "/voice_messages/" + type + "/" + QString::number(chat_id) + "/voiceMessage.wav";
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
