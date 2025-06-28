#include "mainwindow.h"
#include <QtWidgets>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), game(nullptr), networkManager(nullptr), 
      isHost(false), selectedAnswer(-1)
{
    game = new Game(this);
    networkManager = new NetworkManager(this);
    
    setupUI();
    
    // Connect game signals
    connect(game, &Game::gameCreated, this, &MainWindow::onGameCreated);
    connect(game, &Game::playerJoined, this, &MainWindow::onPlayerJoined);
    connect(game, &Game::playerLeft, this, &MainWindow::onPlayerLeft);
    connect(game, &Game::gameStarted, this, &MainWindow::onGameStarted);
    connect(game, &Game::questionChanged, this, &MainWindow::onQuestionChanged);
    connect(game, &Game::answerSubmitted, this, &MainWindow::onAnswerSubmitted);
    connect(game, &Game::allAnswersReceived, this, &MainWindow::onAllAnswersReceived);
    connect(game, &Game::resultsReady, this, &MainWindow::onResultsReady);
    connect(game, &Game::gameEnded, this, &MainWindow::onGameEnded);
    connect(game, &Game::timeUpdate, this, &MainWindow::onTimeUpdate);
    
    // Connect network signals
    connect(networkManager, &NetworkManager::serverStarted, this, &MainWindow::onServerStarted);
    connect(networkManager, &NetworkManager::clientConnected, this, &MainWindow::onClientConnected);
    connect(networkManager, &NetworkManager::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(networkManager, &NetworkManager::connectedToHost, this, &MainWindow::onConnectedToHost);
    connect(networkManager, &NetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(networkManager, &NetworkManager::connectionError, this, &MainWindow::onConnectionError);
    
    showPage(MENU_PAGE);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    setupMenuPage();
    setupCreateGamePage();
    setupJoinGamePage();
    setupLobbyPage();
    setupGamePage();
    setupResultsPage();
    setupFinalResultsPage();
    
    setWindowTitle("QuizzGame");
    resize(800, 600);
}

void MainWindow::setupMenuPage()
{
    menuPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(menuPage);
    
    QLabel* titleLabel = new QLabel("QuizzGame");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #2c3e50; margin: 20px;");
    
    QLabel* nameLabel = new QLabel("Votre nom:");
    playerNameEdit = new QLineEdit();
    playerNameEdit->setPlaceholderText("Entrez votre nom...");
    
    createGameBtn = new QPushButton("Créer une partie");
    joinGameBtn = new QPushButton("Rejoindre une partie");
    
    createGameBtn->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; background-color: #3498db; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #2980b9; }");
    joinGameBtn->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; background-color: #27ae60; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #229954; }");
    
    layout->addWidget(titleLabel);
    layout->addWidget(nameLabel);
    layout->addWidget(playerNameEdit);
    layout->addWidget(createGameBtn);
    layout->addWidget(joinGameBtn);
    layout->addStretch();
    
    connect(createGameBtn, &QPushButton::clicked, this, &MainWindow::onCreateGameClicked);
    connect(joinGameBtn, &QPushButton::clicked, this, &MainWindow::onJoinGameClicked);
    
    stackedWidget->addWidget(menuPage);
}

void MainWindow::setupCreateGamePage()
{
    createGamePage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(createGamePage);
    
    QLabel* titleLabel = new QLabel("Créer une partie");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    
    QLabel* themeLabel = new QLabel("Choisissez un thème:");
    themeComboBox = new QComboBox();
    themeComboBox->addItem("Science", static_cast<int>(Game::SCIENCE));
    themeComboBox->addItem("Sport", static_cast<int>(Game::SPORT));
    themeComboBox->addItem("Culture", static_cast<int>(Game::CULTURE));
    
    createBtn = new QPushButton("Créer la partie");
    backToMenuBtn1 = new QPushButton("Retour");
    
    createBtn->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; background-color: #3498db; color: white; border: none; border-radius: 5px; }");
    backToMenuBtn1->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #95a5a6; color: white; border: none; border-radius: 5px; }");
    
    layout->addWidget(titleLabel);
    layout->addWidget(themeLabel);
    layout->addWidget(themeComboBox);
    layout->addWidget(createBtn);
    layout->addWidget(backToMenuBtn1);
    layout->addStretch();
    
    connect(createBtn, &QPushButton::clicked, [this]() {
        if (playerNameEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez entrer votre nom!");
            return;
        }
        
        currentPlayerName = playerNameEdit->text();
        isHost = true;
        
        Game::Theme theme = static_cast<Game::Theme>(themeComboBox->currentData().toInt());
        game->createGame(theme);
        
        if (!networkManager->startServer()) {
            QMessageBox::critical(this, "Erreur", "Impossible de démarrer le serveur!");
            return;
        }
    });
    connect(backToMenuBtn1, &QPushButton::clicked, this, &MainWindow::onBackToMenuClicked);
    
    stackedWidget->addWidget(createGamePage);
}

void MainWindow::setupJoinGamePage()
{
    joinGamePage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(joinGamePage);
    
    QLabel* titleLabel = new QLabel("Rejoindre une partie");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    
    QLabel* codeLabel = new QLabel("Code de la partie:");
    gameCodeEdit = new QLineEdit();
    gameCodeEdit->setPlaceholderText("Entrez le code à 6 chiffres...");
    
    joinBtn = new QPushButton("Rejoindre");
    backToMenuBtn2 = new QPushButton("Retour");
    
    joinBtn->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; background-color: #27ae60; color: white; border: none; border-radius: 5px; }");
    backToMenuBtn2->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #95a5a6; color: white; border: none; border-radius: 5px; }");
    
    layout->addWidget(titleLabel);
    layout->addWidget(codeLabel);
    layout->addWidget(gameCodeEdit);
    layout->addWidget(joinBtn);
    layout->addWidget(backToMenuBtn2);
    layout->addStretch();
    
    connect(joinBtn, &QPushButton::clicked, [this]() {
        if (playerNameEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez entrer votre nom!");
            return;
        }
        if (gameCodeEdit->text().length() != 6) {
            QMessageBox::warning(this, "Erreur", "Le code doit contenir 6 chiffres!");
            return;
        }
        
        currentPlayerName = playerNameEdit->text();
        isHost = false;
        
        // For simplicity, connect to localhost. In a real app, you'd need the host's IP
        networkManager->connectToHost("127.0.0.1", 12345);
    });
    connect(backToMenuBtn2, &QPushButton::clicked, this, &MainWindow::onBackToMenuClicked);
    
    stackedWidget->addWidget(joinGamePage);
}

void MainWindow::setupLobbyPage()
{
    lobbyPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(lobbyPage);
    
    QLabel* titleLabel = new QLabel("Salle d'attente");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    
    gameCodeLabel = new QLabel();
    gameCodeLabel->setAlignment(Qt::AlignCenter);
    gameCodeLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #e74c3c; margin: 10px;");
    
    QLabel* playersLabel = new QLabel("Joueurs connectés:");
    playerListWidget = new QListWidget();
    
    statusLabel = new QLabel("En attente des joueurs...");
    statusLabel->setAlignment(Qt::AlignCenter);
    
    startGameBtn = new QPushButton("Lancer la partie");
    backToMenuBtn3 = new QPushButton("Retour au menu");
    
    startGameBtn->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; background-color: #e67e22; color: white; border: none; border-radius: 5px; }");
    backToMenuBtn3->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #95a5a6; color: white; border: none; border-radius: 5px; }");
    
    layout->addWidget(titleLabel);
    layout->addWidget(gameCodeLabel);
    layout->addWidget(playersLabel);
    layout->addWidget(playerListWidget);
    layout->addWidget(statusLabel);
    layout->addWidget(startGameBtn);
    layout->addWidget(backToMenuBtn3);
    
    connect(startGameBtn, &QPushButton::clicked, this, &MainWindow::onStartGameClicked);
    connect(backToMenuBtn3, &QPushButton::clicked, this, &MainWindow::onBackToMenuClicked);
    
    stackedWidget->addWidget(lobbyPage);
}

void MainWindow::setupGamePage()
{
    gamePage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(gamePage);
    
    questionCounter = new QLabel();
    questionCounter->setAlignment(Qt::AlignCenter);
    questionCounter->setStyleSheet("font-size: 16px; font-weight: bold;");
    
    timerLabel = new QLabel("Temps restant: 10s");
    timerLabel->setAlignment(Qt::AlignCenter);
    timerLabel->setStyleSheet("font-size: 18px; color: #e74c3c;");
    
    timerProgress = new QProgressBar();
    timerProgress->setRange(0, 10);
    timerProgress->setValue(10);
    
    questionLabel = new QLabel();
    questionLabel->setWordWrap(true);
    questionLabel->setAlignment(Qt::AlignCenter);
    questionLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin: 20px; padding: 20px; background-color: #ecf0f1; border-radius: 10px;");
    
    QWidget* answersWidget = new QWidget();
    QVBoxLayout* answersLayout = new QVBoxLayout(answersWidget);
    
    answer1Btn = new QRadioButton();
    answer2Btn = new QRadioButton();
    answer3Btn = new QRadioButton();
    answer4Btn = new QRadioButton();
    
    QString radioStyle = "QRadioButton { font-size: 16px; padding: 10px; margin: 5px; } QRadioButton::indicator { width: 20px; height: 20px; }";
    answer1Btn->setStyleSheet(radioStyle);
    answer2Btn->setStyleSheet(radioStyle);
    answer3Btn->setStyleSheet(radioStyle);
    answer4Btn->setStyleSheet(radioStyle);
    
    answersLayout->addWidget(answer1Btn);
    answersLayout->addWidget(answer2Btn);
    answersLayout->addWidget(answer3Btn);
    answersLayout->addWidget(answer4Btn);
    
    submitAnswerBtn = new QPushButton("Valider ma réponse");
    submitAnswerBtn->setStyleSheet("QPushButton { padding: 12px; font-size: 16px; background-color: #27ae60; color: white; border: none; border-radius: 5px; }");
    
    waitingLabel = new QLabel("En attente des autres joueurs...");
    waitingLabel->setAlignment(Qt::AlignCenter);
    waitingLabel->setStyleSheet("font-size: 16px; color: #7f8c8d;");
    waitingLabel->hide();
    
    layout->addWidget(questionCounter);
    layout->addWidget(timerLabel);
    layout->addWidget(timerProgress);
    layout->addWidget(questionLabel);
    layout->addWidget(answersWidget);
    layout->addWidget(submitAnswerBtn);
    layout->addWidget(waitingLabel);
    layout->addStretch();
    
    connect(answer1Btn, &QRadioButton::clicked, this, &MainWindow::onAnswerSelected);
    connect(answer2Btn, &QRadioButton::clicked, this, &MainWindow::onAnswerSelected);
    connect(answer3Btn, &QRadioButton::clicked, this, &MainWindow::onAnswerSelected);
    connect(answer4Btn, &QRadioButton::clicked, this, &MainWindow::onAnswerSelected);
    connect(submitAnswerBtn, &QPushButton::clicked, [this]() {
        if (selectedAnswer >= 0) {
            game->submitAnswer(currentPlayerName, selectedAnswer);
            submitAnswerBtn->setEnabled(false);
            waitingLabel->show();
            
            // Send answer to network
            QJsonObject data;
            data["playerName"] = currentPlayerName;
            data["answer"] = selectedAnswer;
            sendNetworkMessage("answer", data);
        }
    });
    
    stackedWidget->addWidget(gamePage);
}

void MainWindow::setupResultsPage()
{
    resultsPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(resultsPage);
    
    QLabel* titleLabel = new QLabel("Résultats de la question");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    
    resultsText = new QTextEdit();
    resultsText->setReadOnly(true);
    resultsText->setStyleSheet("font-size: 14px; background-color: #ecf0f1; border: 1px solid #bdc3c7; border-radius: 5px;");
    
    nextQuestionBtn = new QPushButton("Question suivante");
    backToMenuBtn4 = new QPushButton("Retour au menu");
    
    nextQuestionBtn->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; background-color: #3498db; color: white; border: none; border-radius: 5px; }");
    backToMenuBtn4->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #95a5a6; color: white; border: none; border-radius: 5px; }");
    
    layout->addWidget(titleLabel);
    layout->addWidget(resultsText);
    layout->addWidget(nextQuestionBtn);
    layout->addWidget(backToMenuBtn4);
    
    connect(nextQuestionBtn, &QPushButton::clicked, this, &MainWindow::onNextQuestionClicked);
    connect(backToMenuBtn4, &QPushButton::clicked, this, &MainWindow::onBackToMenuClicked);
    
    stackedWidget->addWidget(resultsPage);
}

void MainWindow::setupFinalResultsPage()
{
    finalResultsPage = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(finalResultsPage);
    
    QLabel* titleLabel = new QLabel("Fin de la partie");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    
    winnerLabel = new QLabel();
    winnerLabel->setAlignment(Qt::AlignCenter);
    winnerLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #f39c12; margin: 20px;");
    
    finalScoresText = new QTextEdit();
    finalScoresText->setReadOnly(true);
    finalScoresText->setStyleSheet("font-size: 14px; background-color: #ecf0f1; border: 1px solid #bdc3c7; border-radius: 5px;");
    
    backToMenuBtn5 = new QPushButton("Retour au menu");
    backToMenuBtn5->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; background-color: #95a5a6; color: white; border: none; border-radius: 5px; }");
    
    layout->addWidget(titleLabel);
    layout->addWidget(winnerLabel);
    layout->addWidget(finalScoresText);
    layout->addWidget(backToMenuBtn5);
    
    connect(backToMenuBtn5, &QPushButton::clicked, this, &MainWindow::onBackToMenuClicked);
    
    stackedWidget->addWidget(finalResultsPage);
}

// Event handlers
void MainWindow::onCreateGameClicked()
{
    showPage(CREATE_GAME_PAGE);
}

void MainWindow::onJoinGameClicked()
{
    showPage(JOIN_GAME_PAGE);
}

void MainWindow::onStartGameClicked()
{
    if (game->getPlayers().size() < 1) {
        QMessageBox::warning(this, "Erreur", "Il faut au moins 1 joueur pour commencer!");
        return;
    }
    
    game->startGame();
    sendNetworkMessage("start_game");
}

void MainWindow::onAnswerSelected()
{
    selectedAnswer = -1;
    if (answer1Btn->isChecked()) selectedAnswer = 0;
    else if (answer2Btn->isChecked()) selectedAnswer = 1;
    else if (answer3Btn->isChecked()) selectedAnswer = 2;
    else if (answer4Btn->isChecked()) selectedAnswer = 3;
    
    submitAnswerBtn->setEnabled(selectedAnswer >= 0);
}

void MainWindow::onNextQuestionClicked()
{
    qDebug() << "=== NEXT QUESTION CLICKED ===";
    qDebug() << "Is host:" << isHost;
    qDebug() << "Current question index before:" << game->getCurrentQuestionIndex();
    qDebug() << "Total questions:" << game->getTotalQuestions();
    
    if (isHost) {
        game->nextQuestion();
        sendNetworkMessage("next_question");
        qDebug() << "Host processed next question";
    } else {
        qDebug() << "ERROR: Non-host tried to click next question!";
    }
    
    qDebug() << "Current question index after:" << game->getCurrentQuestionIndex();
    qDebug() << "=============================";
}
void MainWindow::onBackToMenuClicked()
{
    // Reset everything
    if (networkManager->isServer()) {
        networkManager->stopServer();
    } else {
        networkManager->disconnectFromHost();
    }
    
    playerListWidget->clear();
    selectedAnswer = -1;
    isHost = false;
    currentPlayerName.clear();
    
    showPage(MENU_PAGE);
}

// Game event handlers
void MainWindow::onGameCreated(const QString& code)
{
    game->addPlayer(currentPlayerName);
    gameCodeLabel->setText(QString("Code de la partie: %1").arg(code));
    updatePlayerList();
    showPage(LOBBY_PAGE);
    
    if (isHost) {
        startGameBtn->show();
        statusLabel->setText("Vous êtes l'hôte. Cliquez sur 'Lancer la partie' quand tout le monde est prêt.");
    } else {
        startGameBtn->hide();
        statusLabel->setText("En attente que l'hôte lance la partie...");
    }
}

void MainWindow::onPlayerJoined(const QString& playerName)
{
    updatePlayerList();
}

void MainWindow::onPlayerLeft(const QString& playerName)
{
    updatePlayerList();
}

void MainWindow::onGameStarted()
{
    showPage(GAME_PAGE);
}

void MainWindow::onQuestionChanged(const Question& question)
{
    qDebug() << "=== QUESTION CHANGED ===";
    qDebug() << "New question:" << question.getQuestionText();
    qDebug() << "Question index:" << game->getCurrentQuestionIndex();
    qDebug() << "Is host:" << isHost;
    
    updateGameQuestion();
    
    // Reset UI
    answer1Btn->setChecked(false);
    answer2Btn->setChecked(false);
    answer3Btn->setChecked(false);
    answer4Btn->setChecked(false);
    selectedAnswer = -1;
    submitAnswerBtn->setEnabled(false);
    waitingLabel->hide();
    timerProgress->setValue(10);
    
    // S'assurer qu'on est sur la bonne page
    if (stackedWidget->currentIndex() != GAME_PAGE) {
        qDebug() << "Switching to game page";
        showPage(GAME_PAGE);
    }
    
    qDebug() << "=======================";
}

void MainWindow::onAnswerSubmitted(const QString& playerName, int answer)
{
    // Visual feedback that answer was submitted
}

void MainWindow::onAllAnswersReceived()
{
    // All answers received, waiting for results
}

void MainWindow::onResultsReady(const QMap<QString, bool>& results)
{
    updateResults();
    showPage(RESULTS_PAGE);
    
    // Montrer le bouton seulement pour l'hôte
    if (isHost) {
        nextQuestionBtn->show();
        nextQuestionBtn->setEnabled(true);
    } else {
        nextQuestionBtn->hide();
    }
}

void MainWindow::onGameEnded(const QString& winner)
{
    updateFinalResults();
    showPage(FINAL_RESULTS_PAGE);
}

void MainWindow::onTimeUpdate(int secondsLeft)
{
    timerLabel->setText(QString("Temps restant: %1s").arg(secondsLeft));
    timerProgress->setValue(secondsLeft);
    
    if (secondsLeft <= 3) {
        timerLabel->setStyleSheet("font-size: 18px; color: #e74c3c; font-weight: bold;");
    }
}

// Network event handlers
void MainWindow::onServerStarted(quint16 port)
{
    qDebug() << "Server started on port:" << port;
}

void MainWindow::onClientConnected(const QString& clientId)
{
    qDebug() << "Client connected:" << clientId;
}

void MainWindow::onClientDisconnected(const QString& clientId)
{
    qDebug() << "Client disconnected:" << clientId;
}

void MainWindow::onConnectedToHost()
{
    // Join the game
    QJsonObject data;
    data["playerName"] = currentPlayerName;
    data["gameCode"] = gameCodeEdit->text();
    sendNetworkMessage("join_game", data);
}

void MainWindow::onMessageReceived(const QJsonObject& message, const QString& senderId)
{
    handleNetworkMessage(message, senderId);
}

void MainWindow::onConnectionError(const QString& error)
{
    QMessageBox::critical(this, "Erreur de connexion", error);
}

// Helper methods
void MainWindow::updatePlayerList()
{
    playerListWidget->clear();
    QStringList players = game->getPlayers();
    for (const QString& player : players) {
        playerListWidget->addItem(player);
    }
}

void MainWindow::updateGameQuestion()
{
    Question currentQ = game->getCurrentQuestion();
    questionCounter->setText(QString("Question %1/%2").arg(game->getCurrentQuestionIndex() + 1).arg(game->getTotalQuestions()));
    questionLabel->setText(currentQ.getQuestionText());
    
    QStringList answers = currentQ.getAnswers();
    if (answers.size() >= 4) {
        answer1Btn->setText(answers[0]);
        answer2Btn->setText(answers[1]);
        answer3Btn->setText(answers[2]);
        answer4Btn->setText(answers[3]);
    }
}

void MainWindow::updateResults()
{
    QString resultText = "Résultats de la question:\n\n";
    
    Question currentQ = game->getCurrentQuestion();
    QStringList answers = currentQ.getAnswers();
    
    resultText += QString("Question: %1\n").arg(currentQ.getQuestionText());
    resultText += QString("Bonne réponse: %1\n\n").arg(answers[currentQ.getCorrectAnswerIndex()]);
    
    QMap<QString, int> scores = game->getPlayerScores();
    for (auto it = scores.begin(); it != scores.end(); ++it) {
        resultText += QString("%1: %2 points\n").arg(it.key()).arg(it.value());
    }
    
    resultsText->setText(resultText);
}

void MainWindow::updateFinalResults()
{
    QString winner = game->getWinner();
    winnerLabel->setText(QString("🏆 Gagnant: %1 🏆").arg(winner));
    
    QString scoresText = "Scores finaux:\n\n";
    QMap<QString, int> scores = game->getPlayerScores();
    
    // Sort scores
    QList<QPair<QString, int>> sortedScores;
    for (auto it = scores.begin(); it != scores.end(); ++it) {
        sortedScores.append({it.key(), it.value()});
    }
    
    std::sort(sortedScores.begin(), sortedScores.end(), 
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });
    
    for (int i = 0; i < sortedScores.size(); ++i) {
        scoresText += QString("%1. %2: %3 points\n")
                      .arg(i + 1)
                      .arg(sortedScores[i].first)
                      .arg(sortedScores[i].second);
    }
    
    finalScoresText->setText(scoresText);
}

void MainWindow::showPage(int pageIndex)
{
    stackedWidget->setCurrentIndex(pageIndex);
}

void MainWindow::sendNetworkMessage(const QString& type, const QJsonObject& data)
{
    QJsonObject message;
    message["type"] = type;
    message["data"] = data;
    message["sender"] = currentPlayerName;
    
    networkManager->sendMessage(message);
}

void MainWindow::debugGameState()
{
    qDebug() << "=== DEBUG GAME STATE ===";
    qDebug() << "Current question index:" << game->getCurrentQuestionIndex();
    qDebug() << "Total questions:" << game->getTotalQuestions();
    qDebug() << "Game state:" << game->getState();
    qDebug() << "Is host:" << isHost;
    qDebug() << "Current page:" << stackedWidget->currentIndex();
    qDebug() << "Next question button visible:" << nextQuestionBtn->isVisible();
    qDebug() << "Next question button enabled:" << nextQuestionBtn->isEnabled();
    qDebug() << "========================";
}


void MainWindow::handleNetworkMessage(const QJsonObject& message, const QString& senderId)
{
    QString type = message["type"].toString();
    QJsonObject data = message["data"].toObject();
    
    qDebug() << "Received network message:" << type << "from:" << senderId;
    
    if (type == "join_game") {
        QString playerName = data["playerName"].toString();
        game->addPlayer(playerName);
    }
    else if (type == "start_game") {
        if (!isHost) {
            qDebug() << "Client received start_game message";
            game->startGame();
        }
    }
    else if (type == "answer") {
        QString playerName = data["playerName"].toString();
        int answer = data["answer"].toInt();
        if (isHost) {
            game->submitAnswer(playerName, answer);
        }
    }
    else if (type == "next_question") {
        qDebug() << "Received next_question message, isHost:" << isHost;
        if (!isHost) {
            qDebug() << "Client processing next question";
            game->nextQuestion();
        } else {
            qDebug() << "Host ignoring next_question message (already processed)";
        }
    }
}