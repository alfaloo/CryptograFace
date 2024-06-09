#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QDebug>
#include <QtConcurrent>
#include <QFutureWatcher>

#include <string>
#include <future>

#include "./ui_loginwindow.h"
#include "../headers/loginwindow.h"
#include "../headers/notepadwindow.h"
#include "../headers/facialrecognision.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class LoginWindow;
}
QT_END_NAMESPACE

class LoginWindow : public QMainWindow {
Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    virtual ~LoginWindow();
private:
    Ui::LoginWindow *ui;
    FacialAuthenticator facialAuthenticator;
    void btnLogin_logic(bool showCamera);
    std::queue<std::future<void>> loginAttempts;
signals:
    void loginSuccess(const QString& username);
private slots:
    void on_btnLogin_clicked();
    void openNotepadWindow(const QString& username);
    void on_btnExit_clicked();
    void keyPressEvent(QKeyEvent *event) override;
};
#endif // LOGINWINDOW_H
