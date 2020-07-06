#include "inputer.h"
#include "wiringPi.h"
#include <qdebug.h>
#define RED 1
#define BLACK 5
#define WHITE 27
#define BLUE 26
#define GREEN 7
#define NO -1

Inputer::Inputer(QObject *parent) : QObject(parent)
{
    qDebug()<<"inputer init!";
}

void Inputer::scanInput()
{
    qDebug()<<"start to scan!";
    wiringPiSetup();
    //set five io as input port
    pinMode(RED,INPUT);
    pullUpDnControl(RED,PUD_UP);
    pinMode(BLACK,INPUT);
    pullUpDnControl(BLACK,PUD_UP);
    pinMode(WHITE,INPUT);
    pullUpDnControl(WHITE,PUD_UP);
    pinMode(BLUE,INPUT);
    pullUpDnControl(BLUE,PUD_UP);
    pinMode(GREEN,INPUT);
    pullUpDnControl(GREEN,PUD_UP);

    int res=NO;
    while (1) {
        if(digitalRead(RED)==LOW){
            printf("red pressed\n");
            res=RED;
            delay(50);
        }
        else if (digitalRead(BLACK)==LOW){
            printf("black pressed\n");
            res=BLACK;
            delay(50);
        }
        else if (digitalRead(WHITE)==LOW){
            printf("white pressed\n");
            res=WHITE;
            delay(50);
        }
        else if (digitalRead(BLUE)==LOW){
            printf("blue pressed\n");
            res=BLUE;
            delay(50);
        }
        else if (digitalRead(GREEN)==LOW){
            printf("green pressed\n");
            res=GREEN;
            delay(50);
        }
        emit oneInput(res);
        delay(100);
    }

}
