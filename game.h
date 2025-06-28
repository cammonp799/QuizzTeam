#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QTimer>
#include "question.h"

class Game : public QObject
{
    Q_OBJECT

public:
    enum Theme {
        SCIENCE,
        SPORT,
        CULTURE
    };

    enum GameState {
        WAITING,
        QUESTION_ACTIVE,
        SHOWING_RESULTS,
        GAME_FINISHED
    };

private:
    QString gameCode;
    Theme selectedTheme;
    QVector<Question> questions;
    QMap<QString, int> playerScores;
    QMap<QString, int> currentAnswers;
    int currentQuestionIndex;
    GameState state;
    QTimer* questionTimer;
    bool isHost;

public:
    explicit Game(QObject *parent = nullptr);
    ~Game();
    
    // Game setup
    void createGame(Theme theme);
    QString getGameCode() const;
    void joinGame(const QString& code);
    
    // Player management
    void addPlayer(const QString& playerName);
    void removePlayer(const QString& playerName);
    QStringList getPlayers() const;
    
    // Game flow
    void startGame();
    void nextQuestion();
    void submitAnswer(const QString& playerName, int answerIndex);
    void showResults();
    void endGame();
    
    // Getters
    Question getCurrentQuestion() const;
    int getCurrentQuestionIndex() const;
    int getTotalQuestions() const;
    QMap<QString, int> getPlayerScores() const;
    GameState getState() const;
    QString getWinner() const;
    bool getIsHost() const;
    
    // Static methods
    static QString generateGameCode();
    static QVector<Question> getQuestionsForTheme(Theme theme);

private slots:
    void onTimeUp();

signals:
    void gameCreated(const QString& code);
    void playerJoined(const QString& playerName);
    void playerLeft(const QString& playerName);
    void gameStarted();
    void questionChanged(const Question& question);
    void answerSubmitted(const QString& playerName, int answer);
    void allAnswersReceived();
    void resultsReady(const QMap<QString, bool>& results);
    void gameEnded(const QString& winner);
    void timeUpdate(int secondsLeft);

private:
    void initializeQuestions();
    void checkAllAnswersReceived();
};

#endif // GAME_H