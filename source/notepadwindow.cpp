//
// Created by Zhiyang Lu on 25/05/2024.
//

#include "../headers/notepadwindow.h"
#include "../headers/cryptography.h"
#include "./ui_notepadwindow.h"

NotepadWindow::NotepadWindow(std::string username, QWidget *parent)
        : username(username)
        , QMainWindow(parent)
        , ui(new Ui::NotepadWindow) {
    ui->setupUi(this);
    ui->txtNotepad->setText(QString::fromStdString(decrypt(username)));
}

NotepadWindow::~NotepadWindow() {
    delete ui;
}

void NotepadWindow::on_btnSave_clicked() {
    std::string plain = ui->txtNotepad->toPlainText().toStdString();

    encrypt(username, plain);

    std::cout << "Notepad Saved\n";
}

void NotepadWindow::on_btnExit_clicked() {
    if (decrypt(username) != ui->txtNotepad->toPlainText().toStdString()) {
        QMessageBox::StandardButton reply =
                QMessageBox::question(this,
                                      "Exit Confirmation",
                                      "You have unsaved changes! Do you want to save before closing?",
                                      QMessageBox::Save|QMessageBox::Ignore);

        if (reply == QMessageBox::Save) {
            NotepadWindow::on_btnSave_clicked();
            this->close();
        } else {
            this->close();
        }
    } else {
        this->close();
    }
}