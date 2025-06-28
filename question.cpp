#include "question.h"

// Constructeur par défaut
Question::Question() 
    : correctAnswerIndex(0)
{
}

// Constructeur avec paramètres
Question::Question(const QString& text, const QList<QString>& answers, int correctIndex)
    : questionText(text), answers(answers), correctAnswerIndex(correctIndex)
{
}

// Getter pour le texte de la question
QString Question::getQuestionText() const
{
    return questionText;
}

// Getter pour les réponses
QList<QString> Question::getAnswers() const
{
    return answers;
}

// Getter pour l'index de la bonne réponse
int Question::getCorrectAnswerIndex() const
{
    return correctAnswerIndex;
}

// Vérifier si la réponse est correcte
bool Question::isCorrect(int answerIndex) const
{
    return answerIndex == correctAnswerIndex;
}