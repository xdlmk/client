#include "audiomanager.h"

AudioManager::AudioManager(QObject *parent) : QObject(parent), captureSession(new QMediaCaptureSession(this)),
    recorder(new QMediaRecorder(this)), audioInput(new QAudioInput(this)), player(new QMediaPlayer(this)), audioOutput(new QAudioOutput(this))
{
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(50);
    player->setVideoOutput(nullptr);

    connect(player, &QMediaPlayer::durationChanged, this, &AudioManager::durationChanged);
    connect(player, &QMediaPlayer::positionChanged, this, &AudioManager::positionChanged);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &AudioManager::stateChanged);
    connect(player, &QMediaPlayer::errorOccurred, this, [=](QMediaPlayer::Error error, const QString &errorString) {
        Q_UNUSED(error)
        if (logger) logger->log(Logger::ERROR, "audiomanager.cpp::constructor", "Playback error: " + errorString);
    });
}

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

void AudioManager::playVoice()
{

    if (!player || !audioOutput) {
        qDebug() << "❌ Player or output not initialized";
        return;
    }

    //player->setSource(QUrl::fromLocalFile("C:/c++/qt/chat/clientDes/build/Desktop_Qt_6_9_0_MinGW_64_bit-Debug/.data/9/.voiceFiles/211e0ef5-56f6-4aa6-92a4-2fc4a0c4c094_voiceMessage.wav"));

    player->play();

    if (logger) logger->log(Logger::INFO, "audiomanager.cpp::playVoice", "Playing: " + player->source().toString());
}

void AudioManager::setSource(QString source)
{
    player->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/.voiceFiles/" + source));
}

void AudioManager::setPosition(qint64 position)
{
    player->setPosition(position);
}

void AudioManager::pause()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    }
}

void AudioManager::stop()
{
    player->stop();
    player->setSource(QUrl());
}

void AudioManager::seek(qint64 position)
{
    if (player->playbackState() != QMediaPlayer::StoppedState) {
        player->setPosition(position);
    }
}

QMediaPlayer::PlaybackState AudioManager::getMediaPlayerPlaybackState()
{
    return player->playbackState();
}

qint64 AudioManager::getMediaPlayerDuration()
{
    return player->duration();
}

qint64 AudioManager::getMediaPlayerPosition()
{
    return player->position();
}
