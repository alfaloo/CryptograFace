#include "../headers/loginwindow.h"

namespace fs = std::__fs::filesystem;

LoginWindow::LoginWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::LoginWindow)
        , facialAuthenticator() {
    ui->setupUi(this);
    facialAuthenticator.addLogger(ui->txtInfo);
    connect(this, &LoginWindow::loginSuccess, this, &LoginWindow::openNotepadWindow);
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::on_btnLogin_clicked() {
    if (ui->chbCamera->isChecked()) {
        btnLogin_logic(true);
        return;
    }

    if (loginAttempts.size() > 0) {
        if (loginAttempts.front().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            loginAttempts.pop();
        } else {
            std::string info = "[INFO] Login already in progress.";
            std::cout << info << "\n";

            QString qString = QString::fromStdString(info);
            ui->txtInfo->append(qString);

            ui->txtInfo->moveCursor(QTextCursor::End);
            ui->txtInfo->ensureCursorVisible();

            return;
        }
    }

    std::future<void> future = std::async(
        std::launch::async,
        [this]() { return this->btnLogin_logic(false); }
    );

    loginAttempts.push(std::move(future));
}

void LoginWindow::btnLogin_logic(bool showCamera) {
    std::string username = ui->txtUsername->text().toStdString();

    ui->txtUsername->setText("");

    if (username == "") return;

    facialAuthenticator.uploadUsers("/data/facesets");

    if (!facialAuthenticator.userExists(username)) {
        if (!facialAuthenticator.generateFaceset(username, 1, 6, showCamera)) {
            return;
        }
    }

    if (!facialAuthenticator.trainFaceDescriptors()) {
        return;
    }

    if (facialAuthenticator.authenticate(username, showCamera)) {
        emit loginSuccess(QString::fromStdString(username));
    }
}

void LoginWindow::openNotepadWindow(const QString& username) {
    NotepadWindow* notepadwindow = new NotepadWindow(username.toStdString());
    notepadwindow->show();
    this->close();
}

void LoginWindow::on_btnExit_clicked() {
    this->close();
}

void LoginWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        on_btnLogin_clicked();  // Call the login button click handler directly
    } else {
        std::string key = event->text().toStdString();
        facialAuthenticator.keyPressed = key;
        std::cout << key << "\n";
        QMainWindow::keyPressEvent(event);  // Call the base class implementation for other key presses
    }
}
