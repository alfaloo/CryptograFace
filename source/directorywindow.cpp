//
// Created by Zhiyang Lu on 10/06/2024.
//

#include "../headers/directorywindow.h"

DirectoryWindow::DirectoryWindow(std::string username)
        : username(username) {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    layout = new QGridLayout;
    centralWidget->setLayout(layout);

    QLabel *label = new QLabel("New Entry:");
    QLineEdit *txtName = new QLineEdit();
    QPushButton *btnNewFile = new QPushButton("Text");
    QPushButton *btnNewImage = new QPushButton("Image");

    layout->addWidget(label, 0, 0);
    layout->addWidget(txtName, 0, 1);
    layout->addWidget(btnNewFile, 0, 2);
    layout->addWidget(btnNewImage, 0, 3);

//    connect(btnOpen, &QPushButton::clicked, [this, filename = filename]() {
//        openFile(filename);
//    });
//    connect(btnDelete, &QPushButton::clicked, [this, filename = filename]() {
//        deleteFile(filename);
//    });

    std::string userDir = "/Users/alfaloo/github/CryptograFace/data/ciphers/" + username;

    QDir directory(QString::fromStdString(userDir));
    QStringList textFiles = directory.entryList(QStringList() << "*.txt", QDir::Files);

    int counter = 0;
    foreach (QString file, textFiles) {
        counter++;
        QFileInfo fileInfo(file);
        QString filename = fileInfo.baseName();
        QLabel *label = new QLabel(filename);
        QPushButton *btnOpen = new QPushButton("Open");
        QPushButton *btnDelete = new QPushButton("Delete");

        layout->addWidget(label, counter, 0);
        layout->addWidget(btnOpen, counter, 2);
        layout->addWidget(btnDelete, counter, 3);

        connect(btnOpen, &QPushButton::clicked, [this, filename = filename]() {
            openFile(filename);
        });
        connect(btnDelete, &QPushButton::clicked, [this, filename = filename]() {
            deleteFile(filename);
        });
    }
}

void DirectoryWindow::openFile(const QString& filename) {
    NotepadWindow* notepadwindow = new NotepadWindow(username, filename.toStdString());
    notepadwindow->show();
    this->close();
}

void DirectoryWindow::deleteFile(const QString& filename) {
    std::string cipherPath = "/Users/alfaloo/github/CryptograFace/data/ciphers/" + username + "/" + filename.toStdString() + ".txt";
    QFile cipherFile(QString::fromStdString(cipherPath));

    std::string credentialPath = "/Users/alfaloo/github/CryptograFace/data/credentials/" + username + "/" + filename.toStdString()  + ".txt";
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