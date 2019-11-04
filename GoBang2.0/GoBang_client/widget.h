#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <vector>
#include <set>
#include <QThread>
#include "mythread.h"
#include "aithread.h"
#include "aigobang.hpp"

//const static int dx[] = {1,-1,1,-1,1,-1,0,0};
//const static int dy[] = {1,-1,-1,1,0,0,1,-1};

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) ;
    void drawChessBoard();
    void mousePressEvent(QMouseEvent *event);
    void playChess(QMouseEvent *event);
    void playPVEChess(QMouseEvent *event);
    void AIPlay(int x, int y);
    bool judge(); // 判断输赢
    void dealServerInfo();
    void dealGameTime();
    void dealWindowClose();
    void initChess();
    void setChess(int x, int y, int color);

    void getAiResult(int,int);

    void test();


    void startGameTimer() {
        // 定时器线程
        perPlayTime = 30;
        overTimeCnt = 0;
        thread->start();
        myT->setIsStop(false);
        emit startThread();
    }
    void closeGameTimer() {
        myT->setIsStop(true);
        thread->quit();
        thread->wait();
    }

    void closeAiThread() {
        aiT->quit();
        aiT->wait();
    }
    ~Widget();
    void init();
private slots:



    void on_PVPNetbutton_clicked();

    void on_ConnpushButton_clicked();

    void on_playagainButton_clicked();

    void on_MatchpushButton_clicked();

    void on_pushButton_4_clicked();

    void on_PVEbutton_clicked();

    void drawPVEBoard();

    void on_startGameButton_clicked();





signals:
    void startThread();
    void getAiMove(int,int);
    void setHUMANFirst();
    void setCOMPUTERFirst();
    void aiReset(int level);

private:
    Ui::Widget *ui;
    enum GAMEFORM {PVE,PVPNET,PVPCOMPUTER};
    int gameform;
    int board[30][30];
    int gridWid;
    int gridHei;
    int ChessPieceColor;
    std::vector<std::pair<QPoint, int> > ChessPieces;
    std::set<std::pair<int,int> > isChess;
    bool beginGame = false; // 是否开始游戏
    bool canMove;           // 是否可以下棋
    QString serverIP;
    quint16 serverPORT;
    QString opIP;
    quint16 opPort;
    QTcpSocket* tcpSocket;
    bool ISFIRST;
    int overTimeCnt;

    MyThread* myT;
    QThread* thread;
    int perPlayTime;

    QThread* aiT;
    AiThread* myAi;

    QString tcpId; // 服务器返回的socket编号

    std::pair<int,int> nowChess;

    std::shared_ptr<AiGoBang> ai; // ai
    bool PVEGameBegin;

};

#endif // WIDGET_H
