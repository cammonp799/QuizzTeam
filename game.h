#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QVector>
#include <QMap>
#include "question.h"

class Game : public QObject {
    Q_OBJECT

public:
    Game();
    void startGame();
    void nextQuestion();
    void registerAnswer(const QString &player, int answer);

private:
    QVector<Question> questions;
    QMap<QString, int> scores;
    int currentIndex;
    QMap<QString, bool> answeredPlayers;
};

#endif // GAME_H
