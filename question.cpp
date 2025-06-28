// question.cpp - Corrections
#include "question.h"

Question::Question() 
    : correctAnswerIndex(0)
{
}

Question::Question(const QString& text, const QStringList& answers, int correctIndex)
    : questionText(text), answers(answers), correctAnswerIndex(correctIndex)
{
}

QString Question::getQuestionText() const
{
    return questionText;
}

QStringList Question::getAnswers() const  // Correction du type de retour
{
    return answers;
}

int Question::getCorrectAnswerIndex() const
{
    return correctAnswerIndex;
}

void Question::setQuestionText(const QString& text)
{
    questionText = text;
}

void Question::setAnswers(const QStringList& answerList)
{
    answers = answerList;
}

void Question::setCorrectAnswerIndex(int index)
{
    correctAnswerIndex = index;
}

bool Question::isCorrect(int answerIndex) const
{
    return answerIndex == correctAnswerIndex;
}