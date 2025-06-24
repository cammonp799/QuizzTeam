#ifndef QUESTION_H
#define QUESTION_H

#include <QString>
#include <QStringList>

class Question {
public:
    QString questionText;
    QStringList choices;
    int correctIndex;

    Question(QString q = "", QStringList c = {}, int index = 0)
        : questionText(q), choices(c), correctIndex(index) {}
};

#endif // QUESTION_H
