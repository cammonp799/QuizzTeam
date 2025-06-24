#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "game.h"
#include "networkmanager.h"
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_createGameButton_clicked();
    void on_joinGameButton_clicked();
    void on_startGameButton_clicked();
    void on_answerButton_clicked();

private:
    Ui::MainWindow *ui;
    Game *game;
    NetworkManager *networkManager;
    QButtonGroup *answerGroup; 
};

#endif // MAINWINDOW_H
