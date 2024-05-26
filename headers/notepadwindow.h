//
// Created by Zhiyang Lu on 25/05/2024.
//

#ifndef NOTEPADWINDOW_H
#define NOTEPADWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
    class NotepadWindow;
}
QT_END_NAMESPACE

class NotepadWindow : public QMainWindow
{
Q_OBJECT
private:
    std::string username;
public:
    NotepadWindow(std::string username, QWidget *parent = nullptr);
    ~NotepadWindow();
    Ui::NotepadWindow *ui;
private slots:
    void on_btnSave_clicked();
    void on_btnExit_clicked();
};
#endif // NOTEPADWINDOW_H
