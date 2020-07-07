#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "inputer.h"
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QByteArray>
#include <QString>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStandardItemModel>
#include <QStandardItem>

#define UP 1
#define CONFIRM 5
#define DOWN 27
#define LEFT 26
#define RIGHT 7
#define NO -1

mainWidget::mainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainWidget)
{
    ui->setupUi(this);
    this->ui->label->setText("this is a test label");
    qDebug("init mainWidget");
    //connections
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(sendMessage()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
    connect(ui->checkButton,SIGNAL(clicked),this,SLOT(sendOrders()));
    //init dish map
    initDishMap();
    //init network service
    initTables();

    //start scan input
    Inputer* inputer=new Inputer();
    //set as a sub thread
    inputer->moveToThread(&scanInputThread);
    connect(&scanInputThread,&QThread::finished,inputer,&QObject::deleteLater);
    connect(this,&mainWidget::startScan,inputer,&Inputer::scanInput);
    connect(inputer,&Inputer::oneInput,this,&mainWidget::getOneInput);
    scanInputThread.start();//start sub thread

    emit startScan();

    //set start state as 0
    state=0;
    //start tab page is 0
    currentTab=0;
    currentDish=0;
    ui->tabWidget->setCurrentIndex(0);
    //ui->meatTab->setFocus();
}

void mainWidget::stateSwitch(int input)
{
    qDebug()<<"cur state:"<<state<<" input:"<<input;
    switch (state) {
    //cursor on tab bar
    case 0:
        //if the input is DOWN, change state and break
        if(input==DOWN && currentTab!=ui->tabWidget->count()-1){
            state=1;
            //set focus on the add button in first row
            addBtnMap.find(currentTab).value().at(0)->setFocus();
            break;
        }
        //if not,update current tab index and do not change state
        if(input==RIGHT){
            currentTab+=1;
            if(currentTab>=ui->tabWidget->count()){
                currentTab=0;
            }
        }else if(input==LEFT){
            currentTab-=1;
            if(currentTab<0){
                currentTab=ui->tabWidget->count()-1;
            }
        }
        ui->tabWidget->setCurrentIndex(currentTab);
        //tabChanged(currentTab);
        break;

    //cursor in one table, add button column
    case 1:
        //turn to sub button column
        if(input==RIGHT){
            subBtnMap.find(currentTab).value().at(currentDish)->setFocus();
            state=2;
            break;
        }else if(input==CONFIRM){
            emit addBtnMap.find(currentTab).value().at(currentDish)->clicked();
        }else if(input==DOWN){
            currentDish+=1;
            if(currentDish>=tableList.at(currentTab)->rowCount()){
                currentDish=0;
            }
        }else if(input==UP){
            currentDish-=1;
            if(currentDish<0){
                //currentDish=tableList.at(currentTab)->rowCount()-1;
                //back to tab bar
                currentDish=0;
                ui->tabWidget->setCurrentIndex(currentTab);
                state=0;
                break;
            }
        }
        addBtnMap.find(currentTab).value().at(currentDish)->setFocus();
        break;

    //cursor in sub button column
    case 2:
        if(input==LEFT){
            addBtnMap.find(currentTab).value().at(currentDish)->setFocus();
            state=1;
            break;
        }else if(input==CONFIRM){
            emit subBtnMap.find(currentTab).value().at(currentDish)->clicked();
        }else if(input==DOWN){
            currentDish+=1;
            if(currentDish>=tableList.at(currentTab)->rowCount()){
                currentDish=0;
            }
        }else if(input==UP){
            currentDish-=1;
            if(currentDish<0){
                //currentDish=tableList.at(currentTab)->rowCount()-1;
                //back to tab bar
                currentDish=0;
                ui->tabWidget->setCurrentIndex(currentTab);
                state=0;
                break;
            }
        }
        subBtnMap.find(currentTab).value().at(currentDish)->setFocus();
        break;

    default:
        break;
    }
}

void mainWidget::getOneInput(int i)
{
    this->ui->label->setText(QString::number(i));
    this->stateSwitch(i);//trans state
}

void mainWidget::tabChanged(int cur)
{
    //qDebug()<<"tab count:"<<this->ui->tabWidget->count()<<" clicked tab:"<<cur;
    if(cur==this->ui->tabWidget->count()-1){
        initOrderTable();
    }
}

void mainWidget::initDishMap()
{
    QStringList dishes;
    dishes<<QString::fromLocal8Bit("干煸辣子鸡")
          <<QString::fromLocal8Bit("滋补羊肉锅")
          <<QString::fromLocal8Bit("香煎龙利鱼")
          <<QString::fromLocal8Bit("椒盐玉米虾");
    this->tableList.append(this->ui->meatTableWidget);
    this->dishNameMap.insert(0,dishes);//Meat ends
    dishes.clear();
    dishes<<QString::fromLocal8Bit("山药养生烩")
          <<QString::fromLocal8Bit("清炒芥兰")
          <<QString::fromLocal8Bit("豆角炒茄条")
          <<QString::fromLocal8Bit("蜜汁紫薯");
    this->tableList.append(this->ui->vegeTableWidget);
    this->dishNameMap.insert(1,dishes);//Vegetable ends
    dishes.clear();
    dishes<<QString::fromLocal8Bit("功夫汤")
          <<QString::fromLocal8Bit("罗宋汤")
          <<QString::fromLocal8Bit("蘑菇浓汤");
    this->tableList.append(this->ui->soupTableWidget);
    this->dishNameMap.insert(2,dishes);//Soup ends
    dishes.clear();
    dishes<<QString::fromLocal8Bit("杂粮米饭")
          <<QString::fromLocal8Bit("鸡汤养生面")
          <<QString::fromLocal8Bit("牛肉水饺");
    this->tableList.append(this->ui->stapleTableWidget);
    this->dishNameMap.insert(3,dishes);//Staple ends
    dishes.clear();
    dishes<<QString::fromLocal8Bit("杨枝甘露")
          <<QString::fromLocal8Bit("果蔬拌菜")
          <<QString::fromLocal8Bit("鲜虾小笼包")
          <<QString::fromLocal8Bit("榴莲饼");
    this->tableList.append(this->ui->snackTableWidget);
    this->dishNameMap.insert(4,dishes);//Snack ends
    dishes.clear();
    this->tabCount=5;
}

void mainWidget::initTables()
{
    for(int i=0;i<this->tabCount;++i){
        initTableByIndex(i);
    }
    initOrderTable();
}

void mainWidget::initTableByIndex(int index)
{
    qDebug()<<"init table "<<index;
    QTableWidget* table=this->tableList.at(index);
    //the dish list
    QStringList dishes=this->dishNameMap.find(index).value();
    //init a button list
    QList<QPushButton*> addBtnList;
    QList<QPushButton*> subBtnList;


    //hide headers
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setColumnCount(4);

    //init tables
    for(int i=0;i<dishes.size();++i){
       table->setRowCount(i+1);//reset row count

       //dish name
       table->setItem(i,0,new QTableWidgetItem(dishes.at(i)));

       //dish amount
       QLabel* amtLabel=new QLabel(QString::number(0));
       table->setCellWidget(i,1,amtLabel);

       //add button, set tab index and dish index and added:a bool to show whether added
       QPushButton* addBtn=new QPushButton("添加");
       addBtn->setProperty("tabIndex",index);
       addBtn->setProperty("dishIndex",i);
       addBtn->setProperty("add",1);
       connect(addBtn,SIGNAL(clicked()),this,SLOT(addBtnClicked()));
       table->setCellWidget(i,2,addBtn);

       //sub button
       QPushButton* subBtn=new QPushButton("减少");
       subBtn->setProperty("tabIndex",index);
       subBtn->setProperty("dishIndex",i);
       subBtn->setProperty("add",0);
       connect(subBtn,SIGNAL(clicked()),this,SLOT(addBtnClicked()));
       table->setCellWidget(i,3,subBtn);

       //add buttons to list
       addBtnList.insert(i,addBtn);
       subBtnList.insert(i,subBtn);
    }
    //insert a k-v pair to button map
    addBtnMap.insert(index,addBtnList);
    subBtnMap.insert(index,subBtnList);

}

void mainWidget::initOrderTable()
{
    qDebug()<<"start init order!";
    QTableWidget* table=this->ui->orderTableWidget;
    table->clear();
    //hide headers
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setColumnCount(2);
    //for each dish, add to table
    for(int i=0;i<this->orderList.size();++i){
        table->setRowCount(i+1);
        int dishIndex=orderList.at(i).dishIndex;
        int tabIndex=orderList.at(i).tabIndex;
        //dish name
        qDebug()<<"name: "<<dishNameMap.find(tabIndex).value().at(dishIndex)<<" amount:"<<orderList.at(i).amount;
        table->setItem(i,0,new QTableWidgetItem(this->dishNameMap.find(tabIndex).value().at(dishIndex)));
        //amount
        table->setItem(i,1,new QTableWidgetItem(QString::number(orderList.at(i).amount)));
    }
}

//implement slots
void mainWidget::addBtnClicked()
{
    QPushButton* senderBtn=qobject_cast<QPushButton*>(sender());
    if(senderBtn==NULL){
        return;
    }
    int dishIndex=senderBtn->property("dishIndex").toInt();
    int tabIndex=senderBtn->property("tabIndex").toInt();
    int t=senderBtn->property("add").toInt();
    bool added=(t==1);
    //get the label item
    QLabel* amtLabel=(QLabel*)(this->tableList.at(tabIndex)->cellWidget(dishIndex,1));
    int amt=amtLabel->text().toInt();
    qDebug()<<"dish:"<<dishIndex<<" tab:"<<tabIndex<<" add  :"<<added<<" amount:"<<amt;

    //find the dish index in orderList
    int index=-1;
    for(int i=0;i<orderList.size();++i){
        if(dishIndex==orderList.at(i).dishIndex && tabIndex==orderList.at(i).tabIndex){
            index=i;
            break;
        }
    }
    qDebug()<<index;

    Dish tmp;
    tmp.dishIndex=dishIndex;
    tmp.tabIndex=tabIndex;
    //add amount
    if(added==true){
        amtLabel->setText(QString::number(++amt));
        //no pre, add a new dish to list
        if(index==-1){
            tmp.amount=amt;
            orderList.append(tmp);
        }
        else{
            //orderList.at(index).amount=amt;
            tmp.amount=amt;
            orderList.removeAt(index);//delete the pre one and add new one
            orderList.insert(index,tmp);
        }
    }
    else if(added==false){
        if(amt>0){
            amtLabel->setText(QString::number(--amt));
            if(index!=-1){//has the item
                if(amt==0){//delete one item
                    orderList.removeAt(index);
                }
                else{//sub one amount
                    tmp.amount=amt;
                    orderList.removeAt(index);//delete the pre one and add new one
                    orderList.insert(index,tmp);
                }
            }
        }
    }
}

void mainWidget::sendOrders()
{
    this->sendMessage();
}

void mainWidget::sendMessage()
{
    this->ui->label->setText("clicked");
    QNetworkRequest request;
    request.setUrl(QUrl("http://www.dododawn.com:3888/order"));

    QNetworkAccessManager* netManager=new QNetworkAccessManager (this);
    QMetaObject::Connection connRet = QObject::connect(netManager, SIGNAL(finished(QNetworkReply*)),
                                                       this, SLOT(requestFinished(QNetworkReply*)));
    Q_ASSERT(connRet);

    //request.setUrl();

    //QString mess="{chinese:状态码}";
    Mes mess;
    mess.table=this->ui->tableSpinBox->value();
    QJsonObject json;
    QJsonArray orderJson;
    QJsonArray countJson;
    json.insert("table",mess.table);
    for(int i=0;i<orderList.size();++i){
        mess.order.insert(i,orderList.at(i).tabIndex*10+orderList.at(i).dishIndex);
        mess.count.insert(i,orderList.at(i).amount);

        orderJson.insert(i,QJsonValue(mess.order.at(i)));
        countJson.insert(i,QJsonValue(mess.count.at(i)));
    }
    //mess.str="test string 测试字符串";
    //mess.str=mess.str.toUtf8();
    json.insert("order",QJsonValue(orderJson));
    json.insert("count",QJsonValue(countJson));

    //json.insert("test",mess.str);

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    ui->label->setText(json_str);

    QNetworkReply* reply=netManager->put(request,json_str.toUtf8());//send data
    qDebug()<<json_str.toUtf8();
}

void mainWidget::requestFinished(QNetworkReply* reply)
{
    // 获取http状态码
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        qDebug() << "reason=" << reason.toString();

    QNetworkReply::NetworkError err = reply->error();
    if(err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    }
    else {
        // 获取返回内容
        QByteArray array=reply->readAll();
        QString rev=QString::fromUtf8(array);
        qDebug()<<array;
        qDebug()<<rev;
        //qDebug() << reply->readAll();
    }
}

 mainWidget::~mainWidget(    )
{
    delete ui;
}

