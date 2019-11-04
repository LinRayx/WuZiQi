#ifndef AITHREAD_H
#define AITHREAD_H

#include <QObject>
#include "aigobang.hpp"
#include <memory>
#include <QDebug>
class AiThread : public QObject
{
    Q_OBJECT
public:
    explicit AiThread(QObject *parent = nullptr);
    void run(int,int);
    void setStop(bool flag = true);
    void setHumanFirst() {
        ai->setHUMANFirst();
    }
    void setComputerFirst() {
        ai->setCOMPUTERFirst();
    }
    void reset(int level) {
        qDebug() << "ai reset";
        ai->reset();
        ai->setLevel(level);
    }
private:
    std::shared_ptr<AiGoBang> ai;
    int cnt;

signals:
    void resultGet(int,int);

public slots:
};

#endif // AITHREAD_H
