#include "widget.h"
#include "ui_widget.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QHostAddress>
#include <QDebug>

#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    ui->setupUi(this);
    ui->widget->setFixedSize(550,550);
    ui->MatchpushButton->setEnabled(false);
    ui->GameLabel->setText("<center><h1>五子棋游戏</h1></center");
    ai = std::make_shared<AiGoBang>();
    for(int i = 0; i < 30; ++i) for(int j = 0; j < 30; ++j) board[i][j] = -1;
    beginGame = false;
    ui->stackedWidget->setCurrentIndex(0);
    serverIP = "127.0.0.1";
    serverPORT = 8888;
    perPlayTime = 30;
    tcpSocket = nullptr;
    // 动态分配空间
    myT = new MyThread;
    // 创建子线程
    thread = new QThread(this);

    // 把自定义线程加入到子线程

    myT->moveToThread(thread);

    connect(myT, &MyThread::mySignal,this, &Widget::dealGameTime);

    connect(this, &Widget::startThread, myT, &MyThread::myTimeout);



    aiT = new QThread(this);
    myAi = new AiThread;

    myAi->moveToThread(aiT);


    void(Widget::*getAimove)(int,int) = &Widget::getAiMove;
    void(AiThread::*airun)(int,int) = &AiThread::run;
    void(AiThread::*resultget)(int,int) = &AiThread::resultGet;
    void(Widget::*getairesult)(int,int) = &Widget::getAiResult;


    void(Widget::*aireset)(int) = &Widget::aiReset;
    void(AiThread::*reset)(int) = &AiThread::reset;

    connect(this, getAimove , myAi, airun);

    connect(myAi, resultget, this, getairesult);

    connect(this, &Widget::setHUMANFirst, myAi, &AiThread::setHumanFirst);
    connect(this, &Widget::setCOMPUTERFirst, myAi, &AiThread::setComputerFirst);


    connect(this, aireset, myAi, reset);

    connect(this, &Widget::destroyed, this, &Widget::dealWindowClose);

    ui->FIRSTcomboBox->addItem("玩家先手");
    ui->FIRSTcomboBox->addItem("电脑先手");

    ui->LevelcomboBox->addItem("简单");
    ui->LevelcomboBox->addItem("一般");
    ui->LevelcomboBox->addItem("困难");
    ui->LevelcomboBox->addItem("炼狱");
    qDebug() << "hello";

    aiT->start();
//    test();
}

void Widget::test() {
    emit getAiMove(0,0);
    emit getAiMove(0,0);
}

void Widget::getAiResult(int x, int y) {
//    qDebug() << "主窗口获取ai的结果";
    canMove = true;
    setChess(x+2,y+2,ChessPieceColor^1);
    qDebug() << "ai落子:" << x <<" " << y;
    nowChess.first = x+2; nowChess.second = y+2;
    update();
    bool win = judge();
    if(win) {
        canMove = false;
        qDebug() << "ai赢了";
    }
    else {
        canMove = true;
    }
}

void Widget::dealWindowClose() {
    closeGameTimer();
    closeAiThread();
}

void Widget::dealGameTime() {
    perPlayTime--;
    if(perPlayTime == 0) {

        overTimeCnt++;
        perPlayTime = 30;
        if(overTimeCnt == 2) {
            tcpSocket->write("lose");
            closeGameTimer();
        }
         QMessageBox::information(this,"超时","超时！累计超时两次判定为输！");
    }
    ui->gamelcdNumber->display(perPlayTime);
}
Widget::~Widget()
{
    if(tcpSocket != nullptr) {
        tcpSocket->disconnectFromHost();
        if(tcpSocket->state() == QAbstractSocket::UnconnectedState ||
                tcpSocket->waitForDisconnected(1000))
            qDebug("Disconnected");

    }
    delete ui;
}

void Widget::setChess(int x, int y, int color) {

    ChessPieces.push_back( std::make_pair(QPoint(x,y) , color));
    isChess.insert(std::make_pair(x,y));
    board[x][y] = color;
}
void Widget::paintEvent(QPaintEvent *) {

    if(gameform == GAMEFORM::PVPNET && beginGame == true)
        drawChessBoard();
    else if(gameform == GAMEFORM::PVE && PVEGameBegin == true) {
        drawPVEBoard();
    }
}

void Widget::drawPVEBoard() {
    ui->PVEboard->clear();
    QPainter p(this);
    gridWid = ui->PVEboard->width()/17;
    gridHei = ui->PVEboard->height()/17;
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    p.setPen(pen);
    QBrush brush;
    for(int i = 2; i <= 16; ++i)
        p.drawLine(gridWid*i,gridHei*2, gridWid*i, gridHei*16);
    for(int i = 2; i <= 16; ++i)
        p.drawLine(gridWid*2,gridHei*i, gridWid*16, gridHei*i);
//    qDebug() << ChessPieces.size();
    for(auto& pieces : ChessPieces) {
        if(pieces.second == 0) {
            pen.setColor(Qt::black);
            brush.setColor(Qt::black);

        }
        else {
            pen.setColor(Qt::white);
            brush.setColor(Qt::white);
        }

        brush.setStyle(Qt::SolidPattern);
        p.setPen(pen);
        p.setBrush(brush);

        p.drawEllipse(QPoint(pieces.first.x()*gridWid, pieces.first.y()*gridHei), gridWid/2-5,gridHei/2-5);
//        qDebug() << pieces.first.x() <<" " <<  nowChess.first <<" " << pieces.first.y() <<" "<< nowChess.second;
        if(pieces.first.x() == nowChess.first && pieces.first.y() == nowChess.second) {
            pen.setColor(Qt::green);
            pen.setWidth(1);
            p.setPen(pen);
            p.drawLine(pieces.first.x()*gridWid-12, pieces.first.y()*gridHei-12, pieces.first.x()*gridWid+12, pieces.first.y()*gridHei-12);// -
            p.drawLine(pieces.first.x()*gridWid-12, pieces.first.y()*gridHei-12, pieces.first.x()*gridWid-12, pieces.first.y()*gridHei+12);// |

            p.drawLine(pieces.first.x()*gridWid+12, pieces.first.y()*gridHei+12, pieces.first.x()*gridWid+12, pieces.first.y()*gridHei-12);
            p.drawLine(pieces.first.x()*gridWid+12, pieces.first.y()*gridHei+12, pieces.first.x()*gridWid-12, pieces.first.y()*gridHei+12);
        }
    }

}

void Widget::drawChessBoard() {
    QPainter p(this);
//    ui->stackedWidget->setCurrentIndex(1);

    gridWid = ui->chessBoard->width()/17;
    gridHei = ui->chessBoard->height()/17;


    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    p.setPen(pen);
    QBrush brush;
    for(int i = 2; i <= 16; ++i)
        p.drawLine(gridWid*i,gridHei*2, gridWid*i, gridHei*16);
    for(int i = 2; i <= 16; ++i)
        p.drawLine(gridWid*2,gridHei*i, gridWid*16, gridHei*i);

    for(auto& pieces : ChessPieces) {
        if(pieces.second == 0) {
            pen.setColor(Qt::black);
            brush.setColor(Qt::black);

        }
        else {
            pen.setColor(Qt::white);
            brush.setColor(Qt::white);
        }

        brush.setStyle(Qt::SolidPattern);
        p.setPen(pen);
        p.setBrush(brush);

        p.drawEllipse(QPoint(pieces.first.x()*gridWid, pieces.first.y()*gridHei), gridWid/2-5,gridHei/2-5);

    }
}

void Widget::playChess(QMouseEvent *event) {
    bool flag = false;
    int cor[4][2], pos = 0;
    double mx = 10000000;

    int x = event->x();
    int y = event->y();

    cor[0][0] = x/gridWid;
    cor[0][1] = y/gridHei;

    cor[1][0] = x/gridWid+1;
    cor[1][1] = y/gridHei+1;

    cor[2][0] = x/gridWid+1;
    cor[2][1] = y/gridHei;

    cor[3][0] = x/gridWid;
    cor[3][1] = y/gridHei+1;


    pos = 0;
    mx = 10000000;
    for(int i = 0; i < 4; ++i) {
        double dis = (x-cor[i][0]*gridWid)*(x-cor[i][0]*gridWid)+(y-cor[i][1]*gridHei)*(y-cor[i][1]*gridHei);
        if(dis < mx) {
            mx = dis;
            pos = i;
        }

    }

    if(cor[pos][0] >= 2 && cor[pos][0] <= 16 && cor[pos][1] >= 2 && cor[pos][1] <= 16 && !isChess.count(std::make_pair(cor[pos][0],cor[pos][1]))) {
        ChessPieces.push_back( std::make_pair(QPoint(cor[pos][0], cor[pos][1]) , ChessPieceColor));
        isChess.insert(std::make_pair(cor[pos][0],cor[pos][1]));
        board[cor[pos][0]][cor[pos][1]] = ChessPieceColor;
        flag = true;
    }

    if(flag) {
        bool win = judge();
        if(!win) {
            qDebug() << "落子:" << cor[pos][0] <<" " << cor[pos][1];

            QString playInfo("play#%1#%2");
            playInfo = playInfo.arg(cor[pos][0]).arg(cor[pos][1]);
            qDebug() << playInfo;
            update();
            tcpSocket->write(playInfo.toUtf8().data());
            canMove = false;

            overTimeCnt = -1;
            perPlayTime = 30;
            ui->lcdNumber->display(30);

            ui->playInfoop->setText("轮到你了");
            ui->playInfo->setText("快点下啊！");
        } else {

            tcpSocket->write("win");
            closeGameTimer(); // 关闭线程
            QMessageBox::information(this,"胜利","你赢了!");
            canMove = false;
            ui->playagainButton->setEnabled(true);
        }
    }
}

void Widget::playPVEChess(QMouseEvent *event) {
    bool flag = false;
    int cor[4][2], pos = 0;
    double mx = 10000000;

    int x = event->x();
    int y = event->y();

    cor[0][0] = x/gridWid;
    cor[0][1] = y/gridHei;

    cor[1][0] = x/gridWid+1;
    cor[1][1] = y/gridHei+1;

    cor[2][0] = x/gridWid+1;
    cor[2][1] = y/gridHei;

    cor[3][0] = x/gridWid;
    cor[3][1] = y/gridHei+1;


    pos = 0;
    mx = 10000000;
    for(int i = 0; i < 4; ++i) {
        double dis = (x-cor[i][0]*gridWid)*(x-cor[i][0]*gridWid)+(y-cor[i][1]*gridHei)*(y-cor[i][1]*gridHei);
        if(dis < mx) {
            mx = dis;
            pos = i;
        }

    }

    if(cor[pos][0] >= 2 && cor[pos][0] <= 16 && cor[pos][1] >= 2 && cor[pos][1] <= 16 && !isChess.count(std::make_pair(cor[pos][0],cor[pos][1]))) {
        ChessPieces.push_back( std::make_pair(QPoint(cor[pos][0], cor[pos][1]) , ChessPieceColor));
        isChess.insert(std::make_pair(cor[pos][0],cor[pos][1]));
        board[cor[pos][0]][cor[pos][1]] = ChessPieceColor;
        flag = true;
    }

    if(flag) {
        bool win = judge();
        if(!win) {
            qDebug() << "落子:" << cor[pos][0]-2 <<" " << cor[pos][1]-2;
            nowChess.first = cor[pos][0];
            nowChess.second = cor[pos][1];
            update();
            canMove = false;
            /* ai开始下棋 */
            AIPlay(cor[pos][0]-2 ,cor[pos][1]-2);
        } else {
            canMove = false;
        }
    }
}



void Widget::AIPlay(int x, int y) {
    emit getAiMove(x,y);
//    std::pair<int,int> pos = ai->getCOMPUTERMove(y,x); // 棋盘和ai的相反
//    qDebug() << "ai落子:" << pos.second <<" " << pos.first;

}

void Widget::mousePressEvent(QMouseEvent *event) {
    qDebug() << PVEGameBegin <<" " << canMove;
    if(gameform == GAMEFORM::PVPNET &&beginGame == true && canMove == true)
        playChess(event);
    else if(gameform == GAMEFORM::PVE && PVEGameBegin == true && canMove == true) {
        playPVEChess(event);
    }
}


bool Widget::judge() {
    int dx[] = {1,-1,1,-1,1,-1,0,0};
    int dy[] = {1,-1,-1,1,0,0,1,-1};
    bool flag = false;
    for(auto& it: ChessPieces) {
        int x = it.first.x();
        int y = it.first.y();
//        qDebug() << x <<" " << y;
        int color = it.second;

        for(int j = 0; j < 8; ++j) {
            int cnt = 1;
            int nx = x, ny = y;
            for(int k = 0; k < 4; ++k) {
                nx += dx[j];
                ny += dy[j];
//                qDebug() << board[nx][ny];
                if(nx < 2 || nx > 16 || ny < 2 || ny > 16 || board[nx][ny] != color) {
                    flag = false;
                    break;
                } else cnt++;

            }
//            qDebug() << cnt;
            if(cnt == 5)
                flag = true;
            if(flag)
                break;
        }
    }
    return flag;
}



void Widget::on_PVPNetbutton_clicked()
{
    gameform = GAMEFORM::PVPNET;
    ui->stackedWidget->setCurrentIndex(1);
//    获取服务器ip和端口

}
void Widget::dealServerInfo() {

    QString info = tcpSocket->readAll();
    qDebug() << info;
//    qDebug() << info.mid(4) <<(info.mid(4) == "play");
    if(info.mid(0,13) == "requestOpInfo") {
        QStringList list = info.split('#');
        opIP = list.at(1);
        opPort = list.at(2).toInt();
        ISFIRST = list.at(3).toInt();
        beginGame = true;
        ChessPieceColor = canMove = ISFIRST;
        ui->stackedWidget->setCurrentIndex(2);
        ui->playagainButton->setEnabled(false);
        if(ISFIRST) {
            ui->playInfo->setText("轮到你了");
            ui->playInfoop->setText("快点下啊！");
        }
        else {
            ui->playInfoop->setText("轮到你了");
            ui->playInfo->setText("快点下啊！");
        }

        startGameTimer();

        update();
    } else if(info.mid(0,4) == "play") {
        qDebug() << "recv play";
        QStringList list = info.split('#');

        int opX = list.at(1).toInt();
        int opY = list.at(2).toInt();
        ChessPieces.push_back(std::make_pair(QPoint(opX,opY),!ChessPieceColor));
        canMove = true;
        ui->playInfo->setText("轮到你了");
        ui->playInfoop->setText("快点下啊！");

        overTimeCnt = 0;
        perPlayTime = 30;
        ui->gamelcdNumber->display(perPlayTime);

        update();
    } else if(info == "win") {
        QMessageBox::information(this,"失败","你输了");
        closeGameTimer();
        ui->playagainButton->setEnabled(true);
    } else if(info == "wantplayagain") {
        // 是否再来一把
        qDebug() << "recv playagin";
        int ret = QMessageBox::question(this, tr("My Application"),
                                       tr("对方希望再来一局,是否同意"),
                                       QMessageBox::Yes| QMessageBox::No);

        switch (ret) {
          case QMessageBox::Yes:
                tcpSocket->write("okplayagain"); // b->a
              break;
          case QMessageBox::No:
                tcpSocket->write("noplayagin");
              break;
          default:
              // should never be reached
              break;
        }

    } else if(info == "okplayagain") {
        tcpSocket->write("recokplayagain"); // a->b
        qDebug() << "hello1" << ISFIRST;
        ISFIRST = !ISFIRST;
        initChess();
    } else if(info == "recokplayagain") {
        qDebug() << "hello2" << ISFIRST;
        ISFIRST = !ISFIRST;
        initChess();
    } else if(info.mid(0,7) == "Connect") {
        tcpId = info.split('#').at(1);
    } else if(info == "opUserisdisconnected") {
        initChess();
        ui->stackedWidget->setCurrentIndex(1);
        ui->MatchpushButton->setText("开始匹配");
    }
}

void Widget::init() {
    isChess.clear();
    for(int i = 0; i < 18; ++i) for(int j = 0; j < 18; ++j) board[i][j] = -1;
    ChessPieces.clear();
}
void Widget::on_ConnpushButton_clicked()
{

        if(ui->ConnpushButton->text() == "开始连接") {
            QString ip = ui->IPplainTextEdit->toPlainText();
            quint16 port = ui->PORTplainTextEdit_2->toPlainText().toInt();
            tcpSocket = new QTcpSocket(this);

        //    主动和服务器建立连接
            tcpSocket->connectToHost(QHostAddress(ip), port);

            setWindowTitle("客户端");


            connect(tcpSocket, &QTcpSocket::connected,
                    [=]()
                    {
                        ui->connInfolabel->setText("成功和服务器建立好连接,正在等待对手加入");
                        ui->MatchpushButton->setEnabled(true);
                    }
                    );

            connect(tcpSocket, &QTcpSocket::readyRead,this, &Widget::dealServerInfo);

            ui->ConnpushButton->setText("断开连接");
            QThread::msleep(30);
        } else if(ui->ConnpushButton->text() == "断开连接") {
            tcpSocket->disconnectFromHost();
            if(tcpSocket->state() == QAbstractSocket::UnconnectedState ||
                    tcpSocket->waitForDisconnected(1000)) {
                qDebug("Disconnected");
                ui->ConnpushButton->setText("开始连接");
                QThread::msleep(30);
            }

        }


}

void Widget::initChess() {
    qDebug() << "init";
    overTimeCnt = 0;
    perPlayTime = 30;
    init();
    update();
    beginGame = true;
    canMove = ISFIRST;
    ui->playagainButton->setEnabled(false);
    if(ISFIRST) {
        ui->playInfo->setText("轮到你了");
        ui->playInfoop->setText("快点下啊！");
    }
    else {
        ui->playInfoop->setText("轮到你了");
        ui->playInfo->setText("快点下啊！");
    }

    startGameTimer();
}

void Widget::on_playagainButton_clicked()
{
    int ret = QMessageBox::question(this, tr("My Application"),
                                   tr("再来一局?"),
                                    QMessageBox::No|QMessageBox::Yes);

    switch (ret) {
      case QMessageBox::Yes:
//            qDebug() << ret;
            tcpSocket->write("wantplayagain");
          break;
      case QMessageBox::No:
          break;
      default:
          break;
    }
}

void Widget::on_MatchpushButton_clicked()
{
    if(ui->MatchpushButton->text() == "开始匹配") {
        tcpSocket->write(QString("Match#"+tcpId).toUtf8().data());
        ui->MatchpushButton->setText("取消匹配");
    }
    else if(ui->MatchpushButton->text() == "取消匹配") {
        tcpSocket->write(QString("cancelMatch#"+tcpId).toUtf8().data());
        QThread::msleep(30);
        ui->MatchpushButton->setText("开始匹配");
    }


}

void Widget::on_pushButton_4_clicked()
{
    tcpSocket->disconnectFromHost();
    if(tcpSocket->state() == QAbstractSocket::UnconnectedState ||
            tcpSocket->waitForDisconnected(1000)) {
        qDebug("Disconnected");
        QThread::msleep(30);
        ui->ConnpushButton->setText("开始连接");
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void Widget::on_PVEbutton_clicked()
{
    gameform = GAMEFORM::PVE;
    ui->stackedWidget->setCurrentIndex(4);
}

void Widget::on_startGameButton_clicked()
{

    ChessPieces.clear();
    isChess.clear();
    for(int i = 0; i < 30; ++i) for(int j = 0; j < 30; ++j) board[i][j] = -1;
    update();

    QString Level = ui->LevelcomboBox->currentText();
    int level = 4;
    if(Level == "简单")
        level = 4;
    else if(Level == "一般")
        level = 6;
    else if(Level == "困难")
        level = 7;
    else
        level = 8;

    emit aiReset(level);

    QString FIRST = ui->FIRSTcomboBox->currentText();
    if(FIRST == "玩家先手") {
        emit setHUMANFirst();
    }
    else {
        emit setCOMPUTERFirst();
        setChess(7+2,7+2,ChessPieceColor^1); // 默认走7 7
        update();
    }
    canMove = true;
    PVEGameBegin = true;
}
