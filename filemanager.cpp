#include "filemanager.h"

FileManager::FileManager(QObject *parent)
    : QObject{parent}
{}

QString FileManager::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Select file",
        "~",
        "All files (*)"
        );
    return filePath;
}
