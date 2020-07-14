#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "inputer.h"
#include "post.h"
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
#include <QFile>
#include <QPainter>
#include <QStyleOption>


#define UP 1
#define CONFIRM 5
#define DOWN 27
#define LEFT 26
#define RIGHT 7
#define BACK 25
#define NO -1

mainWidget::mainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainWidget)
{
    ui->setupUi(this);
    this->ui->label->setText("this is a test label");
    this->setFixedSize(564,964);
    qDebug("init mainWidget");

    //load qss
    QString qss;
    QFile qssFile(":/style.qss");
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen())
    {
        qDebug()<<"qss!";
        qss = QLatin1String(qssFile.readAll());
        qDebug()<<qss;
        qApp->setStyleSheet(qss);
        qssFile.close();
    }


    ui->tableLabel->setNum(1);



    //connections
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(sendMessage()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
    connect(ui->checkButton,SIGNAL(clicked()),this,SLOT(sendOrders()));
    connect(ui->addTableButton,SIGNAL(clicked()),this,SLOT(addTableNumber()));
    connect(ui->subTableButton,SIGNAL(clicked()),this,SLOT(subTableNumber()));
    //connect(ui->addTableButton,)
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

    //init POST class
    post=new Post();
    connect(post,&Post::ordersString,this,&mainWidget::receiveOrders);
    connect(post,&Post::menuString,this,&mainWidget::receiveMenu);
    //post->getOrders(12);
    post->getMenu();

    //set start state as 0
    state=0;
    //start tab page is 0
    currentTab=ui->tabWidget->count()-1;
    currentDish=0;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    //ui->meatTab->setFocus();
    ui->addTableButton->setStyleSheet("background-color:rgb(175,238,238)");
}

//receive the menu list and init tables
void mainWidget::receiveMenu(QString str)
{
    if(str==NULL){
        qDebug()<<"GET MENU ERROR";
        return;
    }
    qDebug()<<"Start init menu \n string is :"<<str;
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(str.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        qDebug()<<tr("解析json文件错误！");
        return;
    }

    QJsonObject jsonObject = document.object();
    //qDebug()<<"jsonObject[data]=="<<jsonObject["data"].toString();
    if(jsonObject.contains(QStringLiteral("data")))
    {
        QJsonValue arrayValue = jsonObject.value(QStringLiteral("data"));
        if(arrayValue.isArray())
        {
            QJsonArray array = arrayValue.toArray();
            for(int i=0;i<array.size();i++)
            {
                QJsonValue dishArray = array.at(i);
                QJsonObject dish = dishArray.toObject();
                int id = dish["id"].toInt();
                QString name = dish["name"].toString();
                int type = dish["type"].toInt();
                //int price = dish["price"].toInt();
                QString pr=dish["price"].toString();
                int price=pr.replace("\"","").toInt();
                //qDebug()<<"id="<<id<<"iconTxt="<<iconTxt<<"iconName="<<iconName;
                qDebug()<<"name: "<<name<<" type: "<<type<<" id: "<<id<<" price: "<<price<<" "<<pr;
                Dish tmpD;
                tmpD.id=id;
                tmpD.name=name;
                tmpD.tabIndex=type;
                tmpD.price=price;
                this->menu.insert(i,tmpD);//add to menu list
            }
        }
    }
    initTableHeaders();
    initMenu();
    post->getOrders(1);
}

void mainWidget::initMenu()
{
    for(int i=0;i<menu.size();++i){
        insertDish(menu.at(i));
    }
}

void mainWidget::receiveOrders(QString str)
{
    if(str==NULL){
        qDebug()<<"GET MENU ERROR";
        return;
    }
    //qDebug()<<"rev orders: "<<str;
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(str.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        qDebug()<<tr("解析json文件错误！");
        return;
    }

    preOrder.clear();

    QJsonObject jsonObject = document.object();
    //qDebug()<<"jsonObject[data]=="<<jsonObject["data"].toString();
    if(jsonObject.contains(QStringLiteral("data")))
    {

        QJsonValue arrayValue = jsonObject.value(QStringLiteral("data"));
        if(arrayValue.isArray()){
            QJsonArray array=arrayValue.toArray();
            //qDebug()<<"Array1:"<<array;
            arrayValue=array.at(0);
            jsonObject=arrayValue.toObject();
            if(jsonObject.contains(QStringLiteral("order")))
            {
                arrayValue = jsonObject.value(QStringLiteral("order"));
                if(arrayValue.isArray())
                {
                    QJsonArray array = arrayValue.toArray();
                    for(int i=0;i<array.size();i++)
                    {
                        QJsonValue dishArray = array.at(i);
                        QJsonObject dish = dishArray.toObject();
                        int id = dish["id"].toInt();
                        QString name = dish["name"].toString();
                        //int type = dish["type"].toInt();
                        QString am=dish["number"].toString();
                        int amt=am.replace("\"","").toInt();
                        Order o;
                        o.amount=amt;
                        for(int i=0;i<menu.size();++i){
                            if(id==menu[i].id){
                                o.dish=&menu[i];
                                break;
                            }
                        }
                        qDebug()<<"pre id:"<<id<<" name:"<<name<<" amt:"<<amt;
                        preOrder.append(o);
                    }
                }
            }
        }
    }
    //go back menu, set pre order's amount
    if(preOrder.size()>0){
        for(int i=0;i<preOrder.size();++i){
            Order pre=preOrder.at(i);
            QTableWidget* table=tableList.at(pre.dish->tabIndex);
            for(int j=0;j<table->rowCount();++j){
                // QTableWidgetItem* item=(QTableWidgetItem*)table->cellWidget(j,0);
                 //QString name=item->text();
                QString name=table->item(j,0)->text();
                if(name==pre.dish->name){
                     QLabel* label=(QLabel*)table->cellWidget(j,4);
                     int amt=label->text().toInt();
                     label->setNum(pre.amount);
                 }
            }
        }
    }
    initOrderTable();
}

void mainWidget::stateSwitch(int input)
{
    qDebug()<<"cur state:"<<state<<" input:"<<input;
    switch (state) {
    //cursor on tab bar
    case 0:
        //if the input is DOWN, change state and break
        if(input==DOWN){
            if(currentTab!=ui->tabWidget->count()-1){
                state=1;
                //set focus on the add button in first row
                addBtnMap.find(currentTab).value().at(0)->setFocus();
            }else if(currentTab==ui->tabWidget->count()-1){
                state=3;
                //get in order page
                ui->addTableButton->setFocus();
                ui->addTableButton->setStyleSheet({"background-color:#0099FF"});
            }
            break;
        }else if(input==RIGHT){        //if not,update current tab index and do not change state
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
                currentDish=tableList.at(currentTab)->rowCount()-1;
            }
        }else if(input==BACK){
            state=0;
            ui->tabWidget->setCurrentIndex(currentTab);
            currentDish=0;
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
                currentDish=tableList.at(currentTab)->rowCount()-1;
            }
        }else if(input==BACK){
            state=0;
            ui->tabWidget->setCurrentIndex(currentTab);
        }
        subBtnMap.find(currentTab).value().at(currentDish)->setFocus();
        break;

    //cursor on add table button
    case 3:
        //go to sub button or check button
        if(input==RIGHT){
            ui->subTableButton->setFocus();
            state=4;
        }else if(input==DOWN){
            ui->checkButton->setFocus();
            state=5;
        }else if(input==CONFIRM){
            emit ui->addTableButton->clicked();
        }else if(input==BACK){
            state=0;
            ui->tabWidget->setCurrentIndex(currentTab);
        }
        break;

    //cursor on sub table button
    case 4:
        if(input==LEFT){
            ui->addTableButton->setFocus();
            state=3;
        }else if(input==DOWN){
            ui->checkButton->setFocus();
            state=5;
        }else if(input==CONFIRM){
            emit ui->subTableButton->clicked();
        }else if(input==BACK){
            state=0;
            ui->tabWidget->setCurrentIndex(currentTab);
        }
        break;

    //cursor on check button
    case 5:
        if(input==CONFIRM){
            emit ui->checkButton->clicked();
        }else if(input==UP){
            ui->addTableButton->setFocus();
            state=3;
        }else if(input==BACK){
            state=0;
            ui->tabWidget->setCurrentIndex(currentTab);
        }
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
    this->tableList.append(this->ui->meatTableWidget);
    this->dishNameMap.insert(0,dishes);//Meat ends
    dishes.clear();

    this->tableList.append(this->ui->vegeTableWidget);
    this->dishNameMap.insert(1,dishes);//Vegetable ends
    dishes.clear();

    this->tableList.append(this->ui->soupTableWidget);
    this->dishNameMap.insert(2,dishes);//Soup ends
    dishes.clear();

    this->tableList.append(this->ui->stapleTableWidget);
    this->dishNameMap.insert(3,dishes);//Staple ends
    dishes.clear();

    this->tableList.append(this->ui->snackTableWidget);
    this->tableList.append(this->ui->drinkTableWidget);
    this->dishNameMap.insert(4,dishes);//Snack ends
    dishes.clear();
    this->tabCount=7;
}

void mainWidget::initTables()
{
    for(int i=0;i<this->tabCount;++i){
        //initTableByIndex(i);
        //init
    }
    initOrderTable();
    //inti button map
    for(int i=0;i<this->tabCount-1;++i){
        QList<QPushButton*> adds;
        QList<QPushButton*> subs;
        //insert a k-v pair to button map
        addBtnMap.insert(i,adds);
        subBtnMap.insert(i,subs);
    }
}

void mainWidget::initTableHeaders()
{
    for(int i=0;i<ui->tabWidget->count()-1;++i){
        QTableWidget* table=this->tableList.at(i);
        //hide headers
        table->horizontalHeader()->hide();
        table->verticalHeader()->hide();
        table->setColumnCount(5);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    ui->orderTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void mainWidget::insertDish(Dish d)
{
    int index=d.tabIndex;
    qDebug()<<"insert dish id: "<<d.id<<"at tab: "<<index;
    QTableWidget* table=this->tableList.at(index);
    table->setRowCount(table->rowCount()+1);//append a row
    int i=table->rowCount()-1;
    //dish name
    table->setItem(i,0,new QTableWidgetItem(d.name));
    //dish price
    table->setItem(i,1,new QTableWidgetItem("￥"+QString::number(d.price)));
    //dish amount
    QLabel* amtLabel=new QLabel(QString::number(0));
    amtLabel->setAlignment(Qt::AlignCenter);
    table->setCellWidget(i,4,amtLabel);

    //add button, set tab index and dish index and added:a bool to show whether added
    QPushButton* addBtn=new QPushButton("+");
    addBtn->setProperty("tabIndex",index);
    addBtn->setProperty("dishIndex",i);
    addBtn->setProperty("dishID",d.id);
    addBtn->setProperty("add",1);
    connect(addBtn,SIGNAL(clicked()),this,SLOT(addBtnClicked()));
    table->setCellWidget(i,2,addBtn);

    //sub button
    QPushButton* subBtn=new QPushButton("-");
    subBtn->setProperty("tabIndex",index);
    subBtn->setProperty("dishIndex",i);
    subBtn->setProperty("dishID",d.id);
    subBtn->setProperty("add",0);
    connect(subBtn,SIGNAL(clicked()),this,SLOT(addBtnClicked()));
    table->setCellWidget(i,3,subBtn);

    //add to btn map
    addBtnMap.find(index).value().append(addBtn);
    subBtnMap.find(index).value().append(subBtn);

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
       amtLabel->setAlignment(Qt::AlignCenter);
       table->setCellWidget(i,1,amtLabel);

       //add button, set tab index and dish index and added:a bool to show whether added
       QPushButton* addBtn=new QPushButton("+");
       addBtn->setProperty("tabIndex",index);
       addBtn->setProperty("dishIndex",i);
       addBtn->setProperty("add",1);
       connect(addBtn,SIGNAL(clicked()),this,SLOT(addBtnClicked()));
       table->setCellWidget(i,2,addBtn);

       //sub button
       QPushButton* subBtn=new QPushButton("-");
       subBtn->setProperty("tabIndex",index);
       subBtn->setProperty("dishIndex",i);
       subBtn->setProperty("add",0);
       connect(subBtn,SIGNAL(clicked()),this,SLOT(addBtnClicked()));
       table->setCellWidget(i,3,subBtn);

       //add buttons to list
       addBtnList.insert(i,addBtn);
       subBtnList.insert(i,subBtn);

       //set button style
       //MyStyle::setButtonStyle(addBtn);
       //MyStyle::setButtonStyle(subBtn);
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
    //table->horizontalHeader()->hide();
    QStringList headers;
    headers<<"菜品"<<"单价"<<"数量"<<"总价";
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->hide();
    table->setColumnCount(4);
    table->setRowCount(0);
    qDebug()<<"list size "<<orderList.size()<<" pre:"<<preOrder.size();
    //merge pre order and current order
    for(int i=0;i<preOrder.size();++i){
        int preID=preOrder.at(i).dish->id;
        bool has=false;
        //if has duplicated item
        for(int j=0;j<orderList.size();++j){
            if(preID==orderList.at(j).dish->id){
                Order tmp;
                tmp.dish=orderList.at(j).dish;
                tmp.amount=orderList.at(j).amount+preOrder.at(i).amount;
                orderList.removeAt(j);//delete the pre one and add new one
                orderList.insert(j,tmp);
                has=true;
                break;
            }
        }
        //append orderList
        if(has==false){
            orderList.append(preOrder.at(i));
        }
    }
    preOrder.clear();
    int amtSUM=0;
    int totalSUM=0;
    //for each dish, add to table
    for(int i=0;i<this->orderList.size();++i){
        table->setRowCount(i+1);
        //int dishIndex=orderList.at(i).dishIndex;
        //int tabIndex=orderList.at(i).tabIndex;
        QString name=orderList.at(i).dish->name;
        int amt=orderList.at(i).amount;
        int price=orderList.at(i).dish->price;
        int total=amt*price;
        //dish name
        qDebug()<<"name: "<<name<<" amount:"<<amt;
        table->setItem(i,0,new QTableWidgetItem(name));
        //price
        table->setItem(i,1,new QTableWidgetItem("￥"+QString::number(price)));
        //amount
        table->setItem(i,2,new QTableWidgetItem(QString::number(amt)));
        //total
        table->setItem(i,3,new QTableWidgetItem("￥"+QString::number(total)));
        amtSUM+=amt;
        totalSUM+=total;
    }
    table->setRowCount(table->rowCount()+1);
    table->setItem(table->rowCount()-1,2,new QTableWidgetItem(QString::number(amtSUM)));
    table->setItem(table->rowCount()-1,3,new QTableWidgetItem("￥"+QString::number(totalSUM)));
}

void mainWidget::addTableNumber()
{
    int ct=ui->tableLabel->text().toInt();
    ct=(ct>=15)?1:(ct+1);
    qDebug()<<"table:"<<ct;
    ui->tableLabel->setNum(ct);
    preOrder.clear();
    orderList.clear();
    fetchOrders();
    flushAmt();
}

void mainWidget::subTableNumber()
{
    int ct=ui->tableLabel->text().toInt();
    ct=(ct<=1)?15:(ct-1);
    qDebug()<<"table:"<<ct;
    ui->tableLabel->setNum(ct);
    preOrder.clear();
    orderList.clear();
    fetchOrders();
    flushAmt();
}

void mainWidget::addBtnClicked()
{
    QPushButton* senderBtn=qobject_cast<QPushButton*>(sender());
    if(senderBtn==NULL){
        return;
    }
    int dishID=senderBtn->property("dishID").toInt();
    int dishIndex=senderBtn->property("dishIndex").toInt();
    int tabIndex=senderBtn->property("tabIndex").toInt();
    int t=senderBtn->property("add").toInt();
    bool added=(t==1);
    //get the label item
    QLabel* amtLabel=(QLabel*)(this->tableList.at(tabIndex)->cellWidget(dishIndex,4));
    int amt=amtLabel->text().toInt();
    qDebug()<<"dish:"<<dishIndex<<" tab:"<<tabIndex<<" add  :"<<added<<" amount:"<<amt<<" id:"<<dishID;

    //find the dish index in orderList
    int index=-1;
    for(int i=0;i<orderList.size();++i){
        /*if(dishIndex==orderList.at(i).dishIndex && tabIndex==orderList.at(i).tabIndex){
            index=i;
            break;
        }*/
        //have pre one
        if(dishID==orderList.at(i).dish->id){
            index=i;
            break;
        }
    }
    qDebug()<<index;

    /*Dish tmp;
    tmp.dishIndex=dishIndex;
    tmp.tabIndex=tabIndex;*/
    Order tmp;
    for(int i=0;i<menu.size();++i){
        if(dishID==menu[i].id){
            tmp.dish=&menu[i];
            break;
        }
    }
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
    if(orderList.size()==0){
        return;
    }
    this->sendMessage();
}

void mainWidget::fetchOrders()
{
    int t=ui->tableLabel->text().toInt();
    post->getOrders(t);
}

void mainWidget::sendMessage()
{
    if(orderList.size()==0){
        return;
    }
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
    //mess.table=this->ui->tableSpinBox->value();
    //mess.table=this->ui->tableLabel
    mess.table=this->ui->tableLabel->text().toInt();
    QJsonObject json;
    QJsonArray orderJson;
    QJsonArray countJson;
    json.insert("table",mess.table);
    for(int i=0;i<orderList.size();++i){
        mess.order.insert(i,orderList.at(i).dish->id);
        mess.count.insert(i,orderList.at(i).amount);

        orderJson.insert(i,QJsonValue(mess.order.at(i)));
        countJson.insert(i,QJsonValue(mess.count.at(i)));
    }
    //mess.str="test string 测试字符串";
    //mess.str=mess.str.toUtf8();
    json.insert("order",QJsonValue(orderJson));
    json.insert("number",QJsonValue(countJson));

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

void mainWidget::flushAmt()
{
    for(int i=0;i<tableList.count();++i){
        QTableWidget* table=tableList.at(i);
        for(int j=0;j<table->rowCount();++j){
            QLabel* l=(QLabel*)table->cellWidget(j,4);
            if(l!=nullptr){
                l->setNum(0);
            }
        }
    }
}

void mainWidget::paintEvent(QPaintEvent* )
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&p,this);
    //QWidget::paintEvent(e);
}

mainWidget::~mainWidget(    )
{
    delete ui;
}

