//
// Created by Zhiyang Lu on 10/06/2024.
//

#include "../headers/directorywindow.h"

namespace fs = std::__fs::filesystem;

DirectoryWindow::DirectoryWindow(std::string username)
        : username(username)
        , directoryPath(fs::current_path()) {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    layout = new QGridLayout;
    centralWidget->setLayout(layout);

    QLabel *lblTitle = new QLabel(QString::fromStdString("Welcome back, " + username + "!"));
    layout->addWidget(lblTitle, 0, 0, 1, 4);

    QLabel *lblEntryName = new QLabel("New Entry:");
    txtNewName = new QLineEdit();
    txtNewName->setFixedWidth(142);
    QPushButton *btnNewFile = new QPushButton("Text");
    QPushButton *btnNewImage = new QPushButton("Image");

    layout->addWidget(lblEntryName, 1, 0);
    layout->addWidget(txtNewName, 1, 1, 1, 2);
    layout->addWidget(btnNewFile, 1, 3);
    layout->addWidget(btnNewImage, 1, 4);

    connect(btnNewFile, &QPushButton::clicked, [this]() {
        on_btnNewFile_clicked();
    });
    connect(btnNewImage, &QPushButton::clicked, [this]() {
        on_btnNewImage_clicked();
    });

    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line1, 2, 0, 1, 5);

    QLabel *lblDirectory = new QLabel(QString::fromStdString("Saved Entries:"));
    layout->addWidget(lblDirectory, 3, 0, 1, 5);

    std::string userDir = directoryPath + "/data/ciphers/" + username;

    QDir directory(QString::fromStdString(userDir));
    if (!directory.exists()) {
        directory.mkpath(".");

        std::string userCredentialsDir = directoryPath + "/data/credentials/" + username;
        QDir credentialDirectory(QString::fromStdString(userCredentialsDir));
        credentialDirectory.mkpath(".");
    }

    QStringList textFiles = directory.entryList(QStringList() << "*.txt", QDir::Files);

    int counter = 4;
    foreach (QString file, textFiles) {
        QFileInfo fileInfo(file);
        QString filename = fileInfo.baseName();
        filenames.insert(filename.toStdString());

        QLineEdit *txtFilename = new QLineEdit(filename);
        txtFilename->setFixedWidth(142);
        currentEntryNames.push_back(txtFilename);
        QPushButton *btnRename = new QPushButton("Rename");
        QPushButton *btnOpen = new QPushButton("Open");
        QPushButton *btnDelete = new QPushButton("Delete");

        layout->addWidget(txtFilename, counter, 0, 1, 2);
        layout->addWidget(btnRename, counter, 2);
        layout->addWidget(btnOpen, counter, 3);
        layout->addWidget(btnDelete, counter, 4);

        connect(btnRename, &QPushButton::clicked, [this, currentName = filename.toStdString(), index = counter - 4]() {
            renameFile(currentName, index);
        });
        connect(btnOpen, &QPushButton::clicked, [this, filename = filename]() {
            openFile(filename);
        });
        connect(btnDelete, &QPushButton::clicked, [this, filename = filename]() {
            deleteFile(filename);
        });

        counter++;
    }

    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line2, counter++, 0, 1, 5);

    QPushButton *btnLogout = new QPushButton("Logout");
    QPushButton *btnExit = new QPushButton("Exit");
    layout->addWidget(btnLogout, counter, 3);
    layout->addWidget(btnExit, counter, 4);

    connect(btnLogout, &QPushButton::clicked, [this]() {
        on_btnLogout_clicked();
    });
    connect(btnExit, &QPushButton::clicked, [this]() {
        on_btnExit_clicked();
    });
}

void DirectoryWindow::renameFile(std::string currentName, int index) {
    std::string newName = currentEntryNames[index]->text().toStdString();

    if (newName == "") {
        QMessageBox::information(this, "Information", "File name can not be empty!");
        currentEntryNames[index]->setText(QString::fromStdString(currentName));
        return;
    } else if (filenames.count(newName)) {
        QMessageBox::information(this, "Information", "File name already exists!");
        currentEntryNames[index]->setText(QString::fromStdString(currentName));
        return;
    }

    fs::path oldCipherPath = directoryPath + "/data/ciphers/" + username + "/" + currentName + ".txt";
    fs::path newCipherPath = directoryPath + "/data/ciphers/" + username + "/" + newName + ".txt";

    fs::rename(oldCipherPath, newCipherPath);

    fs::path oldCredentialPath = directoryPath + "/data/credentials/" + username + "/" + currentName + ".txt";
    fs::path newCredentialPath = directoryPath + "/data/credentials/" + username + "/" + newName + ".txt";

    fs::rename(oldCredentialPath, newCredentialPath);

    DirectoryWindow* directorywindow = new DirectoryWindow(username);
    directorywindow->show();
    this->close();
}

void DirectoryWindow::openFile(const QString& filename) {
    NotepadWindow* notepadwindow = new NotepadWindow(username, filename.toStdString());
    notepadwindow->show();
    this->close();
}

void DirectoryWindow::deleteFile(const QString& filename) {
    std::string cipherPath = directoryPath + "/data/ciphers/" + username + "/" + filename.toStdString() + ".txt";
    QFile cipherFile(QString::fromStdString(cipherPath));

    std::string credentialPath = directoryPath + "/data/credentials/" + username + "/" + filename.toStdString()  + ".txt";
    QFile credentialFile(QString::fromStdString(credentialPath));

    if (cipherFile.remove() && credentialFile.remove()) {
        QMessageBox::information(this, "Success", "File deleted successfully");
        DirectoryWindow* directorywindow = new DirectoryWindow(username);
        directorywindow->show();
        this->close();
    } else {
        QMessageBox::warning(this, "Error", "Could not delete file");
        DirectoryWindow* directorywindow = new DirectoryWindow(username);
        directorywindow->show();
        this->close();
    }
}

void DirectoryWindow::on_btnLogout_clicked() {
    LoginWindow* loginwindow = new LoginWindow();
    loginwindow->show();
    this->close();
}

void DirectoryWindow::on_btnExit_clicked() {
    this->close();
}

void DirectoryWindow::on_btnNewFile_clicked() {
    std::string newName = txtNewName->text().toStdString();

    if (newName == "") {
        QMessageBox::information(this, "Information", "File name can not be empty!");
        return;
    } else if (filenames.count(newName)) {
        QMessageBox::information(this, "Information", "File name already exists!");
        txtNewName->setText("");
        return;
    }

    NotepadWindow* notepadwindow = new NotepadWindow(username, newName);
    notepadwindow->show();
    this->close();
}

void DirectoryWindow::on_btnNewImage_clicked() {
    std::cout << "btnNewImage\n";
}