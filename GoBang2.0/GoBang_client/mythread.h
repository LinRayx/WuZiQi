#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>

class MyThread : public QObject
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = nullptr);

    // 线程处理函数
    void myTimeout();
    void setIsStop(bool flag = true) {
        isStop = flag;
    }
signals:
    void mySignal();

public slots:

private:

    bool isStop;
};

#endif // MYTHREAD_H
