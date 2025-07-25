#ifndef QUESTION_H
#define QUESTION_H

#include <QString>
#include <QStringList>

class Question
{
private:
    QString questionText;
    QStringList answers;  // Utiliser QStringList partout
    int correctAnswerIndex;

public:
    Question();
    Question(const QString& text, const QStringList& answerList, int correctIndex);
    
    QString getQuestionText() const;
    QStringList getAnswers() const;  // Cohérent avec le type membre
    int getCorrectAnswerIndex() const;
    
    void setQuestionText(const QString& text);
    void setAnswers(const QStringList& answerList);
    void setCorrectAnswerIndex(int index);
    
    bool isCorrect(int answerIndex) const;
};

#endif // QUESTION_H