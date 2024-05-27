#include <QDebug>
#include "../headers/loginwindow.h"
#include "../headers/notepadwindow.h"
#include "./ui_loginwindow.h"

#include "../headers/facialrecognision.h"

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
    std::string username = ui->txtUsername->toPlainText().toStdString();

    ui->txtUsername->setText("");

    if (username == "") return;

    if (!faceCascade.load(directoryPath + "/data/haarcascades/haarcascade_frontalface_alt.xml")) {
        std::cout << "[Error] Could not load face cascade.\n";
        return;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath + "/data/facesets")) {
        if (fs::is_directory(entry)) {
            currentUsers.insert(entry.path().filename().string());
        }
    }

    if (currentUsers.count(username) == 0) {
        if (!generateFaceset(username, 1, 5)) {
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

