//
// Created by Zhiyang Lu on 10/06/2024.
//

#ifndef CRYPTOGRAFACE_DIRECTORYWINDOW_H
#define CRYPTOGRAFACE_DIRECTORYWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QtWidgets>
#include <QDir>
#include <QFileInfo>

#include "./notepadwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class DirectoryWindow;
}
QT_END_NAMESPACE

class DirectoryWindow : public QMainWindow {
    Q_OBJECT

public:
    DirectoryWindow(std::string username);
private:
    std::string username;
private slots:
    void openFile(const QString& filePath);
    void deleteFile(const QString& filePath);

private:
    QWidget *centralWidget;
    QGridLayout *layout;
};


#endif //CRYPTOGRAFACE_DIRECTORYWINDOW_H
