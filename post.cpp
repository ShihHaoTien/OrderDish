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

Post::Post(QObject *parent) : QObject(parent)
{
    qDebug()<<"post init!";

}

void Post::getMenu()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://dododawn.com:3888/menu_list"));

    QNetworkAccessManager* netManager=new QNetworkAccessManager (this);
    QMetaObject::Connection connRet = QObject::connect(netManager, SIGNAL(finished(QNetworkReply*)),
                                                       this, SLOT(getMenuFinished(QNetworkReply*)));
    Q_ASSERT(connRet);

    QJsonObject json;
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);

    QNetworkReply* reply=netManager->put(request,json_str.toUtf8());//send data
    qDebug()<<"GET MENU:"<<json_str.toUtf8();;
}

//send menu string signal
void Post::getMenuFinished(QNetworkReply *reply)
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
        //qDebug()<<array;
        //qDebug()<<rev;
        //emit ordersString(rev.toUtf8());
        emit menuString(rev.toUtf8());
        return;
        //qDebug() << reply->readAll();
    }
    emit menuString(NULL);

}
void Post::getOrders(int index)
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://dododawn.com:3888/table_order_status"));

    QNetworkAccessManager* netManager=new QNetworkAccessManager (this);
    QMetaObject::Connection connRet = QObject::connect(netManager, SIGNAL(finished(QNetworkReply*)),
                                                       this, SLOT(getOrdersFinished(QNetworkReply*)));
    Q_ASSERT(connRet);

    QJsonObject json;
    json.insert("table",QJsonValue(index));

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);

    QNetworkReply* reply=netManager->put(request,json_str.toUtf8());//send data
    qDebug()<<"GET ORDERS"<<json_str.toUtf8();
}

void Post::getOrdersFinished(QNetworkReply* reply)
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
        //qDebug()<<array;
        qDebug()<<rev;
        emit ordersString(rev.toUtf8());
        return;
        //qDebug() << reply->readAll();
    }
    emit ordersString(NULL);
}
