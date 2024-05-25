#include <QDebug>
#include "../headers/loginwindow.h"
#include "../headers/notepadwindow.h"
#include "./ui_loginwindow.h"

#include "../headers/facialrecognision.h"

namespace fs = std::__fs::filesystem;
std::string directoryPath = fs::current_path();

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

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath + "/facesets")) {
        if (fs::is_directory(entry)) {
            currentUsers.insert(entry.path().filename().string());
        }
    }

    if (currentUsers.count(username) == 0) {
        if (!generateFaceset(username, 5, 10)) {
            std::cout << "[Error] Could not generate facial data.\n";
            return;
        }
    }

    if (!trainFaceset()) {
        std::cout << "[Error] Could not train facial data.\n";
        return;
    }

    if (authenticateFace(username, 30)) {
        NotepadWindow* notepadwindow = new NotepadWindow();
        notepadwindow->show();
        this->close();
    }
}

