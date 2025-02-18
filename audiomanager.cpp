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
    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath()) + "/voiceMessage.wav";
    recorder->setOutputLocation(QUrl::fromLocalFile(filePath));
    recorder->record();
}

void AudioManager::stopRecording() {
    recorder->stop();
}

void AudioManager::playAudio() {
    QString tempPath = QDir::tempPath() + "/voice.wav";
    player->setSource(QUrl::fromLocalFile(tempPath));
    player->play();
    emit playbackStarted();

    connect(player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            emit playbackFinished();
        }
    });
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
