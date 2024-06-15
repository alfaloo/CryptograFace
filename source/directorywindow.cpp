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

    Py_Initialize();
}

DirectoryWindow::~DirectoryWindow() {
    Py_Finalize();
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
    std::pair<std::string, std::string> data = decrypt(username, filename.toStdString());
    if (data.second == "STR") {
        NotepadWindow* notepadwindow = new NotepadWindow(username, filename.toStdString());
        notepadwindow->show();
        this->close();
    } else if (data.second == "IMG") {
        load_canvas_logic(filename.toStdString());
    }
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
    std::string newName = txtNewName->text().toStdString();

    if (newName == "") {
        QMessageBox::information(this, "Information", "File name can not be empty!");
        return;
    } else if (filenames.count(newName)) {
        QMessageBox::information(this, "Information", "File name already exists!");
        txtNewName->setText("");
        return;
    }

    load_canvas_logic(newName);
}

void DirectoryWindow::load_canvas_logic(std::string filename) {
    // Optional: Add the directory containing the Python script to the Python path
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./python_scripts')");

    std::pair<std::string, std::string> content = decrypt(username, filename);
    std::string inCanvas;
    if (content.first != "") inCanvas = content.first;

    // Prepare arguments for Python function
    PyObject* pArgs = PyTuple_New(1);
    PyObject* pCanvasData = PyBytes_FromStringAndSize(inCanvas.data(), inCanvas.size());
    PyTuple_SetItem(pArgs, 0, pCanvasData); // PyTuple_SetItem steals a reference, no need to DECREF pCanvasData

    // Import the module
    PyObject* pName = PyUnicode_DecodeFSDefault("hand_gesture_canvas");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != nullptr) {
        // Get the 'main' function
        PyObject* pFunc = PyObject_GetAttrString(pModule, "main");
        if (pFunc && PyCallable_Check(pFunc)) {
            // Call the function with arguments
            PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
            if (pValue != nullptr) {
                // Check if the result is a bytes object
                if (PyBytes_Check(pValue)) {
                    char* resultCStr = PyBytes_AS_STRING(pValue);
                    std::string outCanvas = std::string(resultCStr, PyBytes_GET_SIZE(pValue));
                    encrypt(username, filename, outCanvas, "IMG");
                } else {
                    std::cerr << "Return value is not a bytes object." << std::endl;
                }
                Py_DECREF(pValue);
            } else {
                PyErr_Print();
                std::cerr << "Call failed\n";
            }
            Py_DECREF(pFunc);
        } else {
            if (PyErr_Occurred())
                PyErr_Print();
            std::cerr << "Cannot find function 'main'\n";
        }
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
        std::cerr << "Failed to load 'hand_gesture_canvas'\n";
    }

    Py_DECREF(pArgs);

    DirectoryWindow* directorywindow = new DirectoryWindow(username);
    directorywindow->show();
    this->close();
}