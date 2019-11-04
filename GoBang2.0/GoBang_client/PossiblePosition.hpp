//
//  PossiblePosition.hpp
//  AiBang
//
//  Created by Linkyrie on 2019/10/31.
//  Copyright © 2019 林恺锐. All rights reserved.
//

#ifndef PossiblePosition_hpp
#define PossiblePosition_hpp
#include <vector>
#include <set>


class PossiblePosition {
    struct History {
        std::vector<std::pair<int,int> > history;
        bool flag;
    };
public:
    
    PossiblePosition(int _WIDTH, int _HEIGHT);
    void updatePosition(int x, int y);
    void addPossiblePosition(int x, int y);
    void rollback(int x, int y);
    std::set<std::pair<int,int> > getCurrentPossiblePos() {return points;}
    void reset();
private:
    
    std::set<std::pair<int,int> > points;
    std::vector<History> hi;
    int WIDTH;
    int HEIGHT;
    bool vis[15][15];
};

#endif /* PossiblePosition_hpp */
