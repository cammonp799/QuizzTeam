#include "game.h"
#include <QDebug>

Game::Game() : currentIndex(0) {
    questions.append(Question("Capital de France?", {"Paris", "Lyon", "Nice", "Toulouse"}, 0));
    questions.append(Question("2 + 2?", {"3", "4", "5", "2"}, 1));
    questions.append(Question("Couleur du ciel?", {"Rouge", "Bleu", "Vert", "Jaune"}, 1));
}

void Game::startGame() {
    currentIndex = 0;
    nextQuestion();
}

void Game::nextQuestion() {
    if (currentIndex < questions.size()) {
        qDebug() << "Question:" << questions[currentIndex].questionText;
        answeredPlayers.clear();
    } else {
        qDebug() << "Fin du jeu. Scores:" << scores;
    }
}

void Game::registerAnswer(const QString &player, int answer) {
    if (answeredPlayers.contains(player)) return;
    answeredPlayers[player] = true;
    if (questions[currentIndex].correctIndex == answer) {
        scores[player] += 1;
    }
    if (answeredPlayers.size() >= 2) {
        currentIndex++;
        nextQuestion();
    }
}
