#ifndef ACMECHINE_H
#define ACMECHINE_H
#include <memory>

class AcMechineNode
{
private:
    std::shared_ptr<AcMechineNode> nxt[3];
    bool ed;
    int score;
    std::shared_ptr<AcMechineNode> fail;
public:
    AcMechineNode();
        
    
    void init() {
        score = 0; ed = false;
        nxt[0] = nxt[1] = nxt[2] = nullptr;
    }
    void set(int pos, std::shared_ptr<AcMechineNode> nxt_node) {
        nxt[pos] = nxt_node;
    }

    std::shared_ptr<AcMechineNode> getNextNode(int pos) {return nxt[pos];}
    void setEnd(bool flag=true) {ed = flag;}
    void setScore(int sc) {score = sc;}
    int getScore(){return score;}
    bool isEnd() {return ed;}
    void setFail( std::shared_ptr<AcMechineNode> pos) {fail = pos;}

    std::shared_ptr<AcMechineNode> getFail() {return fail;}
};

#endif // ACMECHINE_H
