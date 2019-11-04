#include "mythread.h"
#include <QThread>

MyThread::MyThread(QObject *parent) : QObject(parent)
{
    isStop = false;
}


void MyThread::myTimeout() {
    while(isStop == false) {
        QThread::sleep(1);
        emit mySignal();
        if(isStop == true) break;
    }
}
