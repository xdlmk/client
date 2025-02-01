#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class Logger {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    explicit Logger(const QString &logFilePath = "app.log");
    ~Logger();

    void log(LogLevel level, const QString &module, const QString &message);

private:
    QFile logFile;
    QTextStream logStream;

    QString getLevelString(LogLevel level);
};

#endif // LOGGER_H
