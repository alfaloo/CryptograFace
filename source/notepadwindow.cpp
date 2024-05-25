//
// Created by Zhiyang Lu on 25/05/2024.
//

#include "../headers/notepadwindow.h"
#include "./ui_notepadwindow.h"

NotepadWindow::NotepadWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::NotepadWindow)
{
    ui->setupUi(this);
}

NotepadWindow::~NotepadWindow()
{
    delete ui;
}