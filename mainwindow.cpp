#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    game = new Game();
    networkManager = new NetworkManager();

    // Création du QButtonGroup
    answerGroup = new QButtonGroup(this);

    // Supposons que tes boutons réponses sont des QRadioButton nommés answer1RadioButton, answer2RadioButton, etc.
    answerGroup->addButton(ui->answer1RadioButton, 1);
    answerGroup->addButton(ui->answer2RadioButton, 2);
    answerGroup->addButton(ui->answer3RadioButton, 3);
    answerGroup->addButton(ui->answer4RadioButton, 4);
}


MainWindow::~MainWindow() {
    delete ui;
    delete game;
    delete networkManager;
}

void MainWindow::on_createGameButton_clicked() {
    networkManager->startHost(12345);
    QMessageBox::information(this, "Host", "Serveur créé sur le port 12345");
}

void MainWindow::on_joinGameButton_clicked() {
    QString ip = ui->ipLineEdit->text();
    networkManager->connectToHost(ip, 12345);
}

void MainWindow::on_startGameButton_clicked() {
    game->startGame();
}

void MainWindow::on_answerButton_clicked() {
    int selected = answerGroup->checkedId();
    game->registerAnswer("Moi", selected);
}
