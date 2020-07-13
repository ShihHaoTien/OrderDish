#ifndef POST_H
#define POST_H
#include <QObject>
#include <QNetworkReply>
class Post : public QObject
{
    Q_OBJECT
public:
    explicit Post(QObject *parent = nullptr);

    //get the current order, arg is the table number
    void getOrders(int);
    //get the menus
    void getMenu();
private slots:
    void getOrdersFinished(QNetworkReply*);
    void getMenuFinished(QNetworkReply*);

public:
    signals:
    void ordersString(const QString);
};

#endif // POST_H
