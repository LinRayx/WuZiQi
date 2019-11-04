#ifndef AIGOBANG_H
#define AIGOBANG_H

#include <memory>
#include "acmechine.hpp"
#include "PossiblePosition.hpp"
#include <vector>
#include <string>
#include <QDebug>
#include <set>
const static int dx2[] = {1,0,1,-1};  // 下，0，右下，右上
const static int dx1[] = {-1,0,-1,1}; // 上，0，左上，左下
const static int dy2[] = {0,1,1,1};   // 0，右，右下
const static int dy1[] = {0,-1,-1,-1};// 0，左，左上
#define MAX_SCORE 10000000
#define MIN_SCORE -10000000

class AiGoBang
{
    #define HASH_ITEM_INDEX_MASK (0xffff)
    #define UNKNOWN_SCORE (1000000005)
private:
    int WIDTH;
    int HEIGHT;
    int DEPTH;
    
    int Scores[3][72];
    int AllScores[3];
            
    int COMPUTER;
    int HUMEM;
    
    struct PointEva {
        int x,y;
        int eva;
        bool operator < (const PointEva& rhs) const {
            if(eva == rhs.eva) {
                if(x == rhs.x)
                    return y < rhs.y;
                else
                    return x < rhs.x;
            } else
                    return eva > rhs.eva;
                    
        }
    };

    //保存棋局的哈希表条目
    struct HashItem {
        long long checksum;
        int depth;
        int score;
        enum Flag { ALPHA = 0, BETA = 1, EXACT = 2, EMPTY = 3 } flag;
    };

    long long boardZobristValue[2][15][15];
    long long currentZobristValue;
    HashItem hashItems[HASH_ITEM_INDEX_MASK + 1];
                    
    long long random64() {
        return (long long)rand() | ((long long)rand() << 15) | ((long long)rand() << 30) | ((long long)rand() << 45) | ((long long)rand() << 60);
    }
    
    //生成zobrist键值
    void randomBoardZobristValue() {
        int i, j, k;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < WIDTH; j++) {
                for (k = 0; k < WIDTH; k++) {
                    boardZobristValue[i][j][k] = random64();
                }
            }
        }
    }
    void initCurrentZobristValue() {
        currentZobristValue = random64();
    }
                    
    //记录计算结果在哈希表中
    void recordHashItem(int depth, int score, HashItem::Flag flag) {
        int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);
        HashItem *phashItem = &hashItems[index];

        if (phashItem->flag != HashItem::EMPTY && phashItem->depth > depth) {
            return;
        }

        phashItem->checksum = currentZobristValue;
        phashItem->score = score;
        phashItem->flag = flag;
        phashItem->depth = depth;
    }


    //在哈希表中取得计算好的局面的分数
    int getHashItemScore(int depth, int alpha, int beta) {
        int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);
        HashItem *phashItem = &hashItems[index];

        if (phashItem->flag == HashItem::EMPTY)
            return UNKNOWN_SCORE;

        if (phashItem->checksum == currentZobristValue) {
            if (phashItem->depth >= depth) {
                if (phashItem->flag == HashItem::EXACT) {
                    return phashItem->score;
                }
                if (phashItem->flag == HashItem::ALPHA && phashItem->score <= alpha) {
                    return alpha;
                }
                if (phashItem->flag == HashItem::BETA && phashItem->score >= beta) {
                    return beta;
                }
            }
        }

        return UNKNOWN_SCORE;
    }

                    
private:
    
    void getNextPos();

    void show();

    void initBoard() {
//        board.resize(16);
        canPossibleMovePoint.clear();
        blackChess.clear();
        whiteChess.clear();
        for(int i = 0; i < 16; ++i)
            for(int j = 0; j < 16; ++j)
            board[i][j] = 0;
        
    }
    
    bool isEmpty(int i, int j) {
            return board[i][j] == 0;
    }
    void setPos(int x, int y, int color) {
        currentZobristValue ^= boardZobristValue[color - 1][x][y];
        pp->addPossiblePosition(x, y);
        board[x][y] = color;
    }
    
    void clearPos(int x, int y, int color) {
        currentZobristValue ^= boardZobristValue[color - 1][x][y];
        pp->rollback(x, y);
        board[x][y] = 0;
    }
    bool judgeOut(int x, int y) {
        if(x < 0 || x >= HEIGHT || y < 0 || y >= WIDTH)
            return false;
        return true;
    }
    bool haveChess(int x, int y) {
        return board[x][y] != 0;
    }
    
    void initScoreBoard();
    
    void setBlackScoreBoard(const std::string&& s) {
        black_score_board.push_back(s);
        
    }
    void setWhiteScoreBoard(const std::string&& s) {
        white_score_board.push_back(s);
    }
    void addScore(int sc) {
        scores.push_back(sc);
    }

    void insert(std::string&, int sc);
    void buildFail();
    int match(std::string&&, int);
    void extracted(int i, int j, std::string &t);
    
    int Evaluate(int);
    int AlphaBeta(int depth, int alpha, int beta, int);
    int EvaluatePoint(int x, int y, int role);
    
    void updateBoard(std::pair<int,int> p);
                    
    
public:
    /* 开放接口 */
    void test();// 默认人类先下
    AiGoBang();
    std::pair<int,int> getCOMPUTERMove(int x, int y) {
         qDebug() << HUMEM <<" " <<x <<" " << y;
         setPos(x, y, HUMEM);
         getNextPos();
         setPos(searchPos.first,searchPos.second, COMPUTER);
         return searchPos;
    }
    void reset();
    void setLevel(int level = 7) {
        DEPTH = level;
    }
    void setWIDTH(int _WIDTH = 15) {
        WIDTH = _WIDTH;
    }
    void setHEIGHT(int _HEIGHT = 15) {
        HEIGHT = _HEIGHT;
    }
    void setCOMPUTERFirst() {
        COMPUTER = 1;
        HUMEM = 2;
        setPos(7, 7, COMPUTER);
    }
    void setHUMANFirst() {
        COMPUTER = 2;
        HUMEM = 1;
    }
private:
    int board[30][30];
    std::vector<std::string> black_score_board;
    std::vector<std::string> white_score_board;
    std::vector<std::pair<int,int> > whiteChess;
    std::vector<std::pair<int,int> > blackChess;
    std::vector<int> scores;
    std::shared_ptr<AcMechineNode> root;
    std::pair<int,int> searchPos;
    
    std::set<std::pair<int,int> > canPossibleMovePoint;
    std::shared_ptr<PossiblePosition> pp;
};

#endif // AIGOBANG_H
