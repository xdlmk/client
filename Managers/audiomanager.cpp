#include "audiomanager.h"
#include <QTimer>

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

    recordingFilePath = filePath;
    recordingStartTime = QDateTime::currentMSecsSinceEpoch();

    recorder->setOutputLocation(QUrl::fromLocalFile(filePath));
    recorder->record();
}

void AudioManager::stopRecording() {
    recorder->stop();

    qint64 durationMs = QDateTime::currentMSecsSinceEpoch() - recordingStartTime;

    QFile file(recordingFilePath);
    if (file.exists()) {
        QString newFilePath;
        int dotIndex = recordingFilePath.lastIndexOf(".");
        if (dotIndex != -1) {
            newFilePath = recordingFilePath.left(dotIndex) + "_" + QString::number(durationMs) +
                          recordingFilePath.mid(dotIndex);
        } else {
            newFilePath = recordingFilePath + "_" + QString::number(durationMs);
        }
        QFile::rename(recordingFilePath, newFilePath);
    }
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
    QTimer::singleShot(100, [this]() {
        player->setPosition(position);
        player->play();

        if (logger) logger->log(Logger::INFO, "audiomanager.cpp::playVoice", "Playing: " + player->source().toString() + " with position: " + QString::number(player->position()));
    });
}

void AudioManager::setSource(QString source)
{
    player->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/.voiceFiles/" + source));
}

void AudioManager::setPosition(qint64 position)
{
    player->setPosition(position);
    this->position = position;
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
    this->position = 0;
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
