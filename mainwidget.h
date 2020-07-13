#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <qdebug.h>
#include <QTableWidget>
#include <QThread>
#include <QPushButton>
#include "post.h"
namespace Ui {
class mainWidget;
}

struct Mes
{
    //QString str;
    int table;
    QList<int> order;
    QList<int> count;
};

struct Dish
{
    int dishIndex;
    int tabIndex;
    int amount;
};

class mainWidget : public QWidget
{

    Q_OBJECT

public:
    explicit mainWidget(QWidget *parent = nullptr);
    ~mainWidget();

private:
    Ui::mainWidget *ui;
    int tabCount;//the amount of dish type
    QList<Dish> orderList;
    QMap<int,QStringList> dishNameMap;//the map of dishes' name. Key is the tab index, value is a name list.
    QList<QTableWidget*> tableList;//the pointer of each table. Having same index with dishNameMap.
    //button maps, key is page index, value is button list. Use page index and dish index to locate a button.
    QMap<int,QList<QPushButton*>> addBtnMap;
    QMap<int,QList<QPushButton*>> subBtnMap;

    int currentTab;//current tab page index
    int currentDish;//current dish index in one tab page
    QThread scanInputThread;
    int state;//current state in FA

    void initDishMap();
    void initTables();//init the tables in each tab
    void initTableByIndex(int);
    void initOrderTable();
    void stateSwitch(int);//state transfer function, arg is the input signal

    Post* post;

signals:
    void startScan();

private slots:
    //slots
    void sendMessage();
    void addBtnClicked();
    void tabChanged(int);//to determine whether at order page
    void sendOrders();
    void getOneInput(int);

private slots:
    void requestFinished(QNetworkReply*);//get the reply of send orders
    //void receiveOrders(QNetworkReply*);//get the reply of current orders
public slots:
    void receiveOrders(QString);//get the reply of current orders

};

#endif // MAINWIDGET_H
