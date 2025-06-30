#include "game.h"
#include <QRandomGenerator>
#include <QDebug>

Game::Game(QObject *parent)
    : QObject(parent), currentQuestionIndex(0), state(WAITING), isHost(false)
{
    questionTimer = new QTimer(this);
    questionTimer->setSingleShot(true);
    connect(questionTimer, &QTimer::timeout, this, &Game::onTimeUp);
}

Game::~Game()
{
    questionTimer->stop();
}

void Game::createGame(Theme theme)
{
    selectedTheme = theme;
    gameCode = generateGameCode();
    questions = getQuestionsForTheme(theme);
    currentQuestionIndex = 0;
    state = WAITING;
    isHost = true;
    playerScores.clear();
    currentAnswers.clear();
    
    emit gameCreated(gameCode);
}

QString Game::getGameCode() const
{
    return gameCode;
}

void Game::joinGame(const QString& code)
{
    gameCode = code;
    isHost = false;
}

void Game::addPlayer(const QString& playerName)
{
    if (!playerScores.contains(playerName)) {
        playerScores[playerName] = 0;
        emit playerJoined(playerName);
    }
}

void Game::removePlayer(const QString& playerName)
{
    if (playerScores.contains(playerName)) {
        playerScores.remove(playerName);
        currentAnswers.remove(playerName);
        emit playerLeft(playerName);
    }
}

QStringList Game::getPlayers() const
{
    return playerScores.keys();
}

void Game::startGame()
{
    if (questions.isEmpty() || playerScores.isEmpty()) {
        return;
    }
    
    currentQuestionIndex = 0;
    state = QUESTION_ACTIVE;
    currentAnswers.clear();
    
    emit gameStarted();
    emit questionChanged(questions[currentQuestionIndex]);
    
    // Start 10-second timer
    questionTimer->start(10000);
    
    // Emit timer updates
    QTimer* updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);
    int secondsLeft = 10;
    emit timeUpdate(secondsLeft);          // affichage immédiat du « 10 »

    connect(updateTimer, &QTimer::timeout, this,
            [this, updateTimer, secondsLeft]() mutable {
                --secondsLeft;
                emit timeUpdate(secondsLeft);
                if (secondsLeft <= 0) {
                    updateTimer->stop();
                    updateTimer->deleteLater();
                }
            });

    updateTimer->start();

}

void Game::nextQuestion()
{
    currentQuestionIndex++;
    
    if (currentQuestionIndex >= questions.size()) {
        endGame();
        return;
    }
    
    state = QUESTION_ACTIVE;
    currentAnswers.clear();
    
    emit questionChanged(questions[currentQuestionIndex]);
    
    // Start 10-second timer
    questionTimer->start(10000);
    
    // Timer updates
    QTimer* updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);
    int secondsLeft = 10;
    
    connect(updateTimer, &QTimer::timeout, [this, updateTimer, &secondsLeft]() mutable {
        secondsLeft--;
        emit timeUpdate(secondsLeft);
        if (secondsLeft <= 0) {
            updateTimer->stop();
            updateTimer->deleteLater();
        }
    });
    
    updateTimer->start();
}

void Game::submitAnswer(const QString& playerName, int answerIndex)
{
    if (state != QUESTION_ACTIVE || !playerScores.contains(playerName)) {
        return;
    }
    
    currentAnswers[playerName] = answerIndex;
    emit answerSubmitted(playerName, answerIndex);
    
    checkAllAnswersReceived();
}

void Game::showResults()
{
    state = SHOWING_RESULTS;
    questionTimer->stop();
    
    QMap<QString, bool> results;
    Question currentQ = questions[currentQuestionIndex];
    
    // Calculate scores
    for (auto it = playerScores.begin(); it != playerScores.end(); ++it) {
        QString playerName = it.key();
        bool isCorrect = false;
        
        if (currentAnswers.contains(playerName)) {
            int playerAnswer = currentAnswers[playerName];
            isCorrect = currentQ.isCorrect(playerAnswer);
            if (isCorrect) {
                playerScores[playerName]++;
            }
        }
        
        results[playerName] = isCorrect;
    }
    
    emit resultsReady(results);
}

void Game::endGame()
{
    state = GAME_FINISHED;
    questionTimer->stop();
    
    QString winner = getWinner();
    emit gameEnded(winner);
}

Question Game::getCurrentQuestion() const
{
    if (currentQuestionIndex < questions.size()) {
        return questions[currentQuestionIndex];
    }
    return Question();
}

int Game::getCurrentQuestionIndex() const
{
    return currentQuestionIndex;
}

int Game::getTotalQuestions() const
{
    return questions.size();
}

QMap<QString, int> Game::getPlayerScores() const
{
    return playerScores;
}

Game::GameState Game::getState() const
{
    return state;
}

QString Game::getWinner() const
{
    if (playerScores.isEmpty()) {
        return "";
    }
    
    QString winner;
    int maxScore = -1;
    
    for (auto it = playerScores.begin(); it != playerScores.end(); ++it) {
        if (it.value() > maxScore) {
            maxScore = it.value();
            winner = it.key();
        }
    }
    
    return winner;
}

bool Game::getIsHost() const
{
    return isHost;
}

QString Game::generateGameCode()
{
    QString code;
    for (int i = 0; i < 6; ++i) {
        code += QString::number(QRandomGenerator::global()->bounded(10));
    }
    return code;
}

QVector<Question> Game::getQuestionsForTheme(Theme theme)
{
    QVector<Question> themeQuestions;
    
    switch (theme) {
    case SCIENCE:
        themeQuestions.append(Question("Quelle est la formule chimique de l'eau?", 
                                     {"H2O", "CO2", "O2", "NaCl"}, 0));
        themeQuestions.append(Question("Combien de planètes y a-t-il dans notre système solaire?", 
                                     {"7", "8", "9", "10"}, 1));
        themeQuestions.append(Question("Quel est l'élément chimique avec le symbole 'Au'?", 
                                     {"Argent", "Or", "Aluminium", "Argon"}, 1));
        themeQuestions.append(Question("Quelle est la vitesse de la lumière?", 
                                     {"300 000 km/s", "150 000 km/s", "450 000 km/s", "600 000 km/s"}, 0));
        themeQuestions.append(Question("Qui a développé la théorie de la relativité?", 
                                     {"Newton", "Galilée", "Einstein", "Bohr"}, 2));
        break;
        
    case SPORT:
        themeQuestions.append(Question("Combien de joueurs y a-t-il dans une équipe de football?", 
                                     {"10", "11", "12", "9"}, 1));
        themeQuestions.append(Question("Quel pays a gagné la Coupe du Monde 2018?", 
                                     {"Brésil", "Allemagne", "France", "Argentine"}, 2));
        themeQuestions.append(Question("Combien de sets faut-il gagner pour remporter un match de tennis masculin en Grand Chelem?", 
                                     {"2", "3", "4", "5"}, 1));
        themeQuestions.append(Question("Quel sport Michael Jordan a-t-il pratiqué professionnellement?", 
                                     {"Football", "Baseball", "Basketball", "Tennis"}, 2));
        themeQuestions.append(Question("Combien de temps dure un match de rugby?", 
                                     {"80 minutes", "90 minutes", "70 minutes", "60 minutes"}, 0));
        break;
        
    case CULTURE:
        themeQuestions.append(Question("Qui a peint la Joconde?", 
                                     {"Picasso", "Van Gogh", "Leonardo da Vinci", "Monet"}, 2));
        themeQuestions.append(Question("Quelle est la capitale de l'Australie?", 
                                     {"Sydney", "Melbourne", "Canberra", "Perth"}, 2));
        themeQuestions.append(Question("Quel écrivain a créé le personnage de Sherlock Holmes?", 
                                     {"Agatha Christie", "Arthur Conan Doyle", "Edgar Allan Poe", "Charles Dickens"}, 1));
        themeQuestions.append(Question("En quelle année a eu lieu la Révolution française?", 
                                     {"1789", "1792", "1799", "1804"}, 0));
        themeQuestions.append(Question("Quel est le plus long fleuve du monde?", 
                                     {"Amazon", "Nil", "Mississippi", "Yangtsé"}, 1));
        break;
    }
    
    return themeQuestions;
}

void Game::onTimeUp()
{
    showResults();
}

void Game::checkAllAnswersReceived()
{
    if (currentAnswers.size() == playerScores.size()) {
        emit allAnswersReceived();
        showResults();
    }
}
