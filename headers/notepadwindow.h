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

public:
    NotepadWindow(QWidget *parent = nullptr);
    ~NotepadWindow();
    Ui::NotepadWindow *ui;
private slots:

};
#endif // NOTEPADWINDOW_H
