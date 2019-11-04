#include "aithread.h"
#include <QDebug>
#include <QThread>
AiThread::AiThread(QObject *parent) : QObject(parent)
{
    ai = std::make_shared<AiGoBang>();
    cnt = 0;

}

void AiThread::run(int x, int y) {
//    std::pair<int,int> ans = ai->getCOMPUTERMove(cor.first, cor.second);
//    std::pair<int,int> ans(0,0);
    cnt++;
    qDebug() <<QThread::currentThreadId() << "人类落子:" <<cnt << y << x;

    std::pair<int,int> ans = ai->getCOMPUTERMove(y,x);
    emit resultGet(ans.second,ans.first);
}
