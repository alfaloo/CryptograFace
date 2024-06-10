//
// Created by Zhiyang Lu on 10/06/2024.
//

#include "../headers/directorywindow.h"

DirectoryWindow::DirectoryWindow(std::string username)
        : username(username) {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    layout = new QVBoxLayout();
    centralWidget->setLayout(layout);

    std::string userDir = "/Users/alfaloo/github/CryptograFace/data/ciphers/" + username;

    QDir directory(QString::fromStdString(userDir));
    QStringList textFiles = directory.entryList(QStringList() << "*.txt", QDir::Files);
        foreach (QString file, textFiles) {
        QWidget *fileWidget = new QWidget();
        QHBoxLayout *fileLayout = new QHBoxLayout(fileWidget);

        QFileInfo fileInfo(file);
        QString filename = fileInfo.baseName();
        QLabel *label = new QLabel(filename);
        QPushButton *openButton = new QPushButton("Open");
        QPushButton *deleteButton = new QPushButton("Delete");

        fileLayout->addWidget(label);
        fileLayout->addWidget(openButton);
        fileLayout->addWidget(deleteButton);
        layout->addWidget(fileWidget);

        connect(openButton, &QPushButton::clicked, [this, filename = filename]() {
            openFile(filename);
        });
        connect(deleteButton, &QPushButton::clicked, [this, filename = filename]() {
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