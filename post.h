#ifndef POST_H
#define POST_H
#include <QObject>

class Post : public QObject
{
    Q_OBJECT
public:
    explicit Post(QObject *parent = nullptr);

    //get the current order, arg is the table number
    static void getOrders(int);
    //get the menus
    static void getMenu();
private slots:
    void getOrdersFinished();

public:
    signals:
    void ordersString(const QString);
};

#endif // POST_H
