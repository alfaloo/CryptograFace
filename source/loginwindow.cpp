#include "../headers/loginwindow.h"

namespace fs = std::__fs::filesystem;

LoginWindow::LoginWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_btnLogin_clicked()
{
    std::string username = ui->txtUsername->text().toStdString();

    ui->txtUsername->setText("");

    if (username == "") return;

    if (!faceCascade.load(directoryPath + "/data/haarcascades/haarcascade_frontalface_alt.xml")) {
        std::cout << "[Error] Could not load face cascade.\n";
        return;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath + "/data/facesets")) {
        if (fs::is_directory(entry) && fs::is_empty(entry)) {
            fs::remove(entry.path());  // Deletes the directory
            std::cout << "[Info] Deleted empty directory: " << entry.path() << "\n";
        } else if (fs::is_directory(entry)) {
            currentUsers.insert(entry.path().filename().string());
        }
    }

    if (currentUsers.count(username) == 0) {
        if (!generateFaceset(username, 1, 6)) {
            std::cout << "[Error] Could not generate facial data.\n";
            return;
        }
    }

    if (!trainFaceDescriptors()) {
        std::cout << "[Error] Could not train facial data.\n";
        return;
    }

    if (authenticate(username)) {
        NotepadWindow* notepadwindow = new NotepadWindow(username);
        notepadwindow->show();
        this->close();
    }
}

void LoginWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        on_btnLogin_clicked();  // Call the login button click handler directly
    } else {
        QMainWindow::keyPressEvent(event);  // Call the base class implementation for other key presses
    }
}
