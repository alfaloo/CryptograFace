#include "../headers/loginwindow.h"

namespace fs = std::__fs::filesystem;

LoginWindow::LoginWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::LoginWindow)
        , facialAuthenticator() {
    ui->setupUi(this);
    facialAuthenticator.addLogger(ui->txtInfo);
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::on_btnLogin_clicked() {
    std::string username = ui->txtUsername->text().toStdString();

    ui->txtUsername->setText("");

    if (username == "") return;

    facialAuthenticator.uploadUsers("/data/facesets");

    if (!facialAuthenticator.userExists(username)) {
        if (!facialAuthenticator.generateFaceset(username, 1, 6)) {
            return;
        }
    }

    if (!facialAuthenticator.trainFaceDescriptors()) {
        return;
    }

    if (facialAuthenticator.authenticate(username)) {
        NotepadWindow* notepadwindow = new NotepadWindow(username);
        notepadwindow->show();
        this->close();
    }
}

void LoginWindow::on_btnExit_clicked() {
    this->close();
}

void LoginWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        on_btnLogin_clicked();  // Call the login button click handler directly
    } else {
        QMainWindow::keyPressEvent(event);  // Call the base class implementation for other key presses
    }
}
