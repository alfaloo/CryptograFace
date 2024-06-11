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
#include <QMessageBox>

#include <stdio.h>
#include <vector>
#include <unordered_set>

#include "./notepadwindow.h"
#include "./loginwindow.h"

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
    void on_btnNewFile_clicked();
    void on_btnNewImage_clicked();
    void renameFile(std::string currentName, int index);
    void openFile(const QString& filePath);
    void deleteFile(const QString& filePath);
    void on_btnLogout_clicked();
    void on_btnExit_clicked();

private:
    std::unordered_set<std::string> filenames;
    std::vector<QLineEdit*> currentEntryNames;
    QLineEdit *txtNewName;
    QWidget *centralWidget;
    QGridLayout *layout;
    std::string directoryPath;
};


#endif //CRYPTOGRAFACE_DIRECTORYWINDOW_H
