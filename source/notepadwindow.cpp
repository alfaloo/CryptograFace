//
// Created by Zhiyang Lu on 25/05/2024.
//

#include "../headers/notepadwindow.h"

NotepadWindow::NotepadWindow(std::string username, std::string filename, QWidget *parent)
        : username(username)
        , filename(filename)
        , QMainWindow(parent)
        , ui(new Ui::NotepadWindow) {
    ui->setupUi(this);
    ui->txtNotepad->setText(QString::fromStdString(decrypt(username, filename)));
}

NotepadWindow::~NotepadWindow() {
    delete ui;
}

void NotepadWindow::on_btnSave_clicked() {
    std::string plain = ui->txtNotepad->toPlainText().toStdString();

    encrypt(username, filename, plain);

    std::cout << "Notepad Saved\n";
}

void NotepadWindow::on_btnBack_clicked() {
    autosave(Leave::back);
}

void NotepadWindow::on_btnExit_clicked() {
    autosave(Leave::quit);
}

void NotepadWindow::autosave(Leave option) {
    if (decrypt(username, filename) != ui->txtNotepad->toPlainText().toStdString()) {
        QMessageBox::StandardButton reply =
                QMessageBox::question(this,
                                      "Exit Confirmation",
                                      "You have unsaved changes! Do you want to save before closing?",
                                      QMessageBox::Save|QMessageBox::Ignore);

        if (reply == QMessageBox::Save) {
            NotepadWindow::on_btnSave_clicked();
            if (option == Leave::back) {
                DirectoryWindow* directorywindow = new DirectoryWindow(username);
                directorywindow->show();
            }
            this->close();
        } else {
            if (option == Leave::back) {
                DirectoryWindow* directorywindow = new DirectoryWindow(username);
                directorywindow->show();
            }
            this->close();
        }
    } else {
        if (option == Leave::back) {
            DirectoryWindow* directorywindow = new DirectoryWindow(username);
            directorywindow->show();
        }
        this->close();
    }
}