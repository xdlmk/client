#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QUuid>
#include <QDebug>
#include <QFileDialog>

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);

    Q_INVOKABLE QString openFile();


};

#endif // FILEMANAGER_H
