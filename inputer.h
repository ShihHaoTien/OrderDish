#ifndef INPUTER_H
#define INPUTER_H

#include <QObject>

//check whether has input signal
class Inputer : public QObject
{
    Q_OBJECT
public:
    explicit Inputer(QObject *parent = nullptr);

signals:
    void oneInput(const int);

public slots:
    void scanInput();
};

#endif // INPUTER_H
