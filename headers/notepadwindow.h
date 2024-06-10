//
// Created by Zhiyang Lu on 25/05/2024.
//

#ifndef NOTEPADWINDOW_H
#define NOTEPADWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>

#include "./cryptography.h"
#include "./ui_notepadwindow.h"
#include "./directorywindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class NotepadWindow;
}
QT_END_NAMESPACE

enum Leave {
    back,
    quit,
};

class NotepadWindow : public QMainWindow {
Q_OBJECT
private:
    std::string username;
    std::string filename;
    void autosave(Leave option);
public:
    NotepadWindow(std::string username, std::string filename, QWidget *parent = nullptr);
    ~NotepadWindow();
    Ui::NotepadWindow *ui;
private slots:
    void on_btnSave_clicked();
    void on_btnBack_clicked();
    void on_btnExit_clicked();
};
#endif // NOTEPADWINDOW_H
