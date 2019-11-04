#include "aigobang.hpp"
#include <vector>
#include <queue>
#include <iostream>
#include <memory>
AiGoBang::AiGoBang()
{
    root = std::make_shared<AcMechineNode>();
    memset(Scores, 0, sizeof Scores);
    memset(AllScores, 0, sizeof AllScores);

    DEPTH = 8;
    WIDTH = 15; HEIGHT = 15;

    pp = std::make_shared<PossiblePosition>(WIDTH, HEIGHT);
    initBoard();
    initScoreBoard();
    random64();
    randomBoardZobristValue();
    currentZobristValue = random64();
    
}

void AiGoBang::reset() {
    qDebug() << "reset";
    initBoard();
    pp->reset();
    memset(Scores, 0, sizeof Scores);
    memset(AllScores, 0, sizeof AllScores);
    random64();
    randomBoardZobristValue();
    currentZobristValue = random64();
}

int AiGoBang::EvaluatePoint(int x, int y, int role) {
    int i = x , j = y, sum = 0;
    
    for(int k = 0; k < 4; ++k) {
        int len=0;
        std::string t;
        int ni1 = i, nj1 = j;
        for(int l = 0; l < 4; ++l){
           ni1 += dx1[k];nj1 += dy1[k];
           if(!judgeOut(ni1, nj1)) break;
           t += board[ni1][nj1]+'0';

        }
        
        std::reverse(t.begin(), t.end());
        len = int(t.size());
        t += role+'0';

        int ni2 = i, nj2 = j;
        for(int l = 0; l < 4; ++l){
           ni2 += dx2[k]; nj2 += dy2[k];
           if(!judgeOut(ni2,nj2)) break;
           t += board[ni2][nj2]+'0';
        }
        sum += match(std::move(t), role);
        t[len] = (3-role)+'0';
        sum += match(std::move(t), 3-role);
    }
    
    return sum;
}



int AiGoBang::AlphaBeta(int depth, int alpha, int beta, int role) {
//    show();
    HashItem::Flag flag = HashItem::ALPHA;
    int score = getHashItemScore(depth, alpha, beta);
    if (score != UNKNOWN_SCORE && depth != DEPTH) {
        return score;
    }
    
    int sc1 = Evaluate(role);
    int sc2 = Evaluate(3-role);
    
    if(sc1 >= 50000) {
        return 80000-1000+(DEPTH-depth);
    }
    if(sc2 >= 50000) {
        return -80000+1000+(DEPTH-depth);
    }
    if(depth == 0) {
        recordHashItem(depth, sc1 - sc2, HashItem::EXACT);
        return sc1-sc2;
    }
    
    std::set<PointEva> prioMovePoint;
    std::set<std::pair<int,int> > tmp = pp->getCurrentPossiblePos();
    for(auto& p: tmp) {
//        std::cout << p.first <<" " << p.second << std::endl;
        prioMovePoint.insert({p.first,p.second,EvaluatePoint(p.first, p.second, HUMEM)});
     
    }
    int cnt = 0;
    
    while(prioMovePoint.empty() == false){
        auto p = *prioMovePoint.begin();
        prioMovePoint.erase(prioMovePoint.begin());
        int i = p.x, j = p.y;
        
//        std::cout << p.x <<" " << p.y <<" " << p.eva << std::endl;
        if(i == 4 && j == 8) {
//            std::cout<<"";
        }
        if(board[i][j] == 0) {
            cnt++;
            
            setPos(i, j, role);
            
            updateBoard(std::make_pair(i, j));
           
            int val = -AlphaBeta(depth-1, -beta, -alpha, 3-role);
//            if (depth == DEPTH)
//                std::cout << "score(" << p.x << "," << p.y << "):" << val << std::endl;
            clearPos(i,j,role);
            
            updateBoard(std::make_pair(i, j));
            if(val >= beta) {
                recordHashItem(depth, beta, HashItem::BETA);
                return beta;
            }
            if(val > alpha) {
                alpha = val;
                flag = HashItem::EXACT;
                if(depth == DEPTH) {
                    searchPos = std::make_pair(i,j);
                }
            }
            if(cnt == 10) break;
        }
    }
    recordHashItem(depth, alpha, flag);
    return alpha;
}

void AiGoBang::getNextPos() {
    AlphaBeta(DEPTH, -100000000, 100000000, COMPUTER);
//    std::cout << searchPos.first << " " << searchPos.second << std::endl;
}

/* ac自动机模块 begin */
void AiGoBang::insert(std::string & str, int sc) {
    int len = int(str.length());
    std::shared_ptr<AcMechineNode> tmp = root;
    for(int i = 0; i < len; ++i) {
        int num = int(str[i]-'0');
        if(tmp->getNextNode(num) != nullptr) {
            tmp = tmp->getNextNode(num);
        } else {
            std::shared_ptr<AcMechineNode> nAc(new AcMechineNode);
            tmp->set(num, nAc);
            tmp = nAc;
        }
    }
    tmp->setEnd(true);
    tmp->setScore(sc);
}

void AiGoBang::buildFail() {
    std::queue<std::shared_ptr<AcMechineNode> > que;
    root->setFail(nullptr);
    for(int i = 0; i < 3; ++i) {
        if(root->getNextNode(i) != nullptr) {
            que.push(root->getNextNode(i));
            root->getNextNode(i)->setFail(root);
        } else root->set(i, root);

    }


    while (!que.empty()) {
        std::shared_ptr<AcMechineNode> tmp = que.front();
        que.pop();

        for(int i = 0; i < 3; ++i) {
            std::shared_ptr<AcMechineNode> nt = tmp->getNextNode(i);
            if(nt == nullptr) {
                tmp->set(i, tmp->getFail()->getNextNode(i));

            } else {
                nt->setFail(tmp->getFail()->getNextNode(i));
                que.push(nt);
            }
        }
    }
}

int AiGoBang::match(std::string && str, int color) {
    int sum = 0;
    int len = int(str.length());
    std::shared_ptr<AcMechineNode> now = root;
    for(int i = 0; i < len; ++i) {
        int num = str[i]-'0';
        if(color == 2 && num != 0) num = 3-num; // 白棋
        now = now->getNextNode(num);
        std::shared_ptr<AcMechineNode> tmp = now;
        while(tmp != nullptr) {
            if(tmp->isEnd()) {
                sum += tmp->getScore();
//                std::cout << i <<" " << tmp->getScore() << std::endl;
            }
            
            tmp = tmp->getFail();
        }
    }
    return sum;
}

/* ac自动机模块end */

// black 1 white 2
void AiGoBang::initScoreBoard() {

    setBlackScoreBoard("11111");
    addScore(50000);
    //2
    setBlackScoreBoard("011110");
    addScore(4320);
    //3
    setBlackScoreBoard("011100");
    addScore(720);
    //4
    setBlackScoreBoard("001110");
    addScore(720);
    //5
    setBlackScoreBoard("011010");
    addScore(720);
    //6
    setBlackScoreBoard("010110");
    addScore(720);
    //7
    setBlackScoreBoard("11110");
    addScore(720);
    //8
    setBlackScoreBoard("01111");
    addScore(720);
    //9
    setBlackScoreBoard("11011");
    addScore(720);
    //10
    setBlackScoreBoard("10111");
    addScore(720);
    //11
    setBlackScoreBoard("11101");
    addScore(720);
    //12
    setBlackScoreBoard("001100");
    addScore(120);
    //13
    setBlackScoreBoard("001010");
    addScore(120);
    //14
    setBlackScoreBoard("010100");
    addScore(120);
    //15
    setBlackScoreBoard("000100");
    addScore(20);
    //16
    setBlackScoreBoard("001000");
    addScore(20);
    for(int i = 0; i < black_score_board.size(); ++i) {
        insert(black_score_board[i], scores[i]); // 1
    }
    buildFail();
}

void AiGoBang::test() {
//    match("0110101010111011011111011211110", 1);
//    setPos(7, 7, 3-color);
//    match("000000210000000",1);
    setHUMANFirst();
    while(1) {
        show();
        int nx, ny;
        std::cout << "请输入下棋位置";
        std::cin >> nx >> ny;
        setPos(nx,ny,HUMEM);
        updateBoard(std::make_pair(nx, ny));
        blackChess.push_back(std::make_pair(nx, ny));
        getNextPos();
        whiteChess.push_back(searchPos);
        setPos(searchPos.first,searchPos.second, COMPUTER);
        updateBoard(std::make_pair(searchPos.first,searchPos.second));
    }
}

void AiGoBang::show() {
    for(int i = 0; i < 15; ++i) {
        for(int j = 0; j < 15; ++j) {
            if(board[i][j] == 0)
                std::cout <<"-";
            else if(board[i][j] == 1)
                std::cout <<"o";
            else if(board[i][j] == 2)
                std::cout <<"*";
        }
        std::cout << "\n";
    }
}

void AiGoBang::updateBoard(std::pair<int, int> p) {
    
    std::string line1[4];
    std::string line2[4];
    int x = p.first, y = p.second;
    for(int i = 0; i < WIDTH; ++i) {

        line1[0] += board[x][i]+'0'; // -
        line2[0] += board[x][i]+'0';
        line1[1] += board[i][y]+'0'; // |
        line2[1] += board[i][y]+'0';
        
    }
    
    for(int i = x-std::min(x,y), j = y-std::min(x,y); i < WIDTH && j < WIDTH; ++i, ++j) {
        line1[2] += board[i][j]+'0';
        line2[2] += board[i][j]+'0';
    }
    
    for(int i = x+std::min(y,WIDTH-1-x), j = y-std::min(y,WIDTH-1-x); i >= 0 && j < WIDTH; --i, ++j) {
        line1[3] += board[i][j]+'0';
        line2[3] += board[i][j]+'0';
    }
        
    int line1score[4], line2score[4];
    for(int i = 0; i < 4; ++i) {
        line1score[i] = match(std::move(line1[i]),1);
        line2score[i] = match(std::move(line2[i]),2);
    }
    
    // - | \ /
    int a = x;
    int b = WIDTH+y;
    int c = 2*WIDTH+(y-x+10);
    int d = 2*WIDTH+21+(x+y-4);
    
    
    for(int i = 0; i < 2; ++i) {
        AllScores[i] -= Scores[i][a];
        AllScores[i] -= Scores[i][b];
    }
    
    Scores[0][a] = line1score[0];
    Scores[0][b] = line1score[1];
    
    
    Scores[1][a] = line2score[0];
    Scores[1][b] = line2score[1];
    
    
    for(int i = 0; i < 2; ++i) {
        AllScores[i] += Scores[i][a];
        AllScores[i] += Scores[i][b];
    }
    if(y-x >= -10 && y-x <= 10) {
        for(int i = 0; i < 2; ++i) {
            AllScores[i] -= Scores[i][c];
        }
        Scores[0][c] = line1score[2];
        Scores[1][c] = line2score[2];
        for(int i = 0; i < 2; ++i) {
            AllScores[i] += Scores[i][c];
        }
    }
    
    if(x+y >= 4 && x+y <= 24) {
        for(int i = 0; i < 2; ++i) {
            AllScores[i] -= Scores[i][d];
        }
        Scores[0][d] = line1score[3];
        Scores[1][d] = line2score[3];
        for(int i = 0; i < 2; ++i) {
            AllScores[i] += Scores[i][d];
        }
    }
}

int AiGoBang::Evaluate(int role) {
    if(role == 1) return AllScores[0];
    return AllScores[1];
}
