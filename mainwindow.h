#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QProgressBar>
#include <QListWidget>
#include <QJsonObject>
#include "game.h"
#include "networkmanager.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QRadioButton;
class QVBoxLayout;
class QHBoxLayout;
class QStackedWidget;
class QComboBox;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI Slots
    void onCreateGameClicked();
    void onJoinGameClicked();
    void onStartGameClicked();
    void onAnswerSelected();
    void onNextQuestionClicked();
    void onBackToMenuClicked();
    
    // Game Slots
    void onGameCreated(const QString& code);
    void onPlayerJoined(const QString& playerName);
    void onPlayerLeft(const QString& playerName);
    void onGameStarted();
    void onQuestionChanged(const Question& question);
    void onAnswerSubmitted(const QString& playerName, int answer);
    void onAllAnswersReceived();
    void onResultsReady(const QMap<QString, bool>& results);
    void onGameEnded(const QString& winner);
    void onTimeUpdate(int secondsLeft);
    
    // Network Slots
    void onServerStarted(quint16 port);
    void onClientConnected(const QString& clientId);
    void onClientDisconnected(const QString& clientId);
    void onConnectedToHost();
    void onMessageReceived(const QJsonObject& message, const QString& senderId);
    void onConnectionError(const QString& error);

private:
    // UI Setup
    void setupUI();
    void setupMenuPage();
    void setupCreateGamePage();
    void setupJoinGamePage();
    void setupLobbyPage();
    void setupGamePage();
    void setupResultsPage();
    void setupFinalResultsPage();
    void debugGameState();
    
    // UI Updates
    void updatePlayerList();
    void updateGameQuestion();
    void updateResults();
    void updateFinalResults();
    void showPage(int pageIndex);
    
    // Network messaging
    void sendNetworkMessage(const QString& type, const QJsonObject& data = QJsonObject());
    void handleNetworkMessage(const QJsonObject& message, const QString& senderId);
    
    // Core objects
    Game* game;
    NetworkManager* networkManager;
    
    // UI Components
    QStackedWidget* stackedWidget;
    
    // Menu page
    QWidget* menuPage;
    QPushButton* createGameBtn;
    QPushButton* joinGameBtn;
    QLineEdit* playerNameEdit;
    
    // Create game page
    QWidget* createGamePage;
    QComboBox* themeComboBox;
    QPushButton* createBtn;
    QPushButton* backToMenuBtn1;
    
    // Join game page
    QWidget* joinGamePage;
    QLineEdit* gameCodeEdit;
    QPushButton* joinBtn;
    QPushButton* backToMenuBtn2;
    
    // Lobby page
    QWidget* lobbyPage;
    QLabel* gameCodeLabel;
    QListWidget* playerListWidget;
    QPushButton* startGameBtn;
    QPushButton* backToMenuBtn3;
    QLabel* statusLabel;
    
    // Game page
    QWidget* gamePage;
    QLabel* questionLabel;
    QLabel* questionCounter;
    QLabel* timerLabel;
    QProgressBar* timerProgress;
    QRadioButton* answer1Btn;
    QRadioButton* answer2Btn;
    QRadioButton* answer3Btn;
    QRadioButton* answer4Btn;
    QPushButton* submitAnswerBtn;
    QLabel* waitingLabel;
    
    // Results page
    QWidget* resultsPage;
    QTextEdit* resultsText;
    QPushButton* nextQuestionBtn;
    QPushButton* backToMenuBtn4;
    
    // Final results page
    QWidget* finalResultsPage;
    QLabel* winnerLabel;
    QTextEdit* finalScoresText;
    QPushButton* backToMenuBtn5;
    
    // State variables
    QString currentPlayerName;
    bool isHost;
    int selectedAnswer;
    QTimer* uiUpdateTimer;
    
    enum PageIndex {
        MENU_PAGE = 0,
        CREATE_GAME_PAGE = 1,
        JOIN_GAME_PAGE = 2,
        LOBBY_PAGE = 3,
        GAME_PAGE = 4,
        RESULTS_PAGE = 5,
        FINAL_RESULTS_PAGE = 6
    };
};

#endif // MAINWINDOW_H