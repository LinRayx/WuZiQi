//
//  PossiblePosition.cpp
//  AiBang
//
//  Created by Linkyrie on 2019/10/31.
//  Copyright © 2019 林恺锐. All rights reserved.
//

#include "PossiblePosition.hpp"




PossiblePosition::PossiblePosition(int _WIDTH, int _HEIGHT) {
    memset(vis, 0, sizeof vis);
    points.clear();

    WIDTH = _WIDTH;
    HEIGHT = _HEIGHT;
}

void PossiblePosition::reset() {
    memset(vis, 0, sizeof vis);
    points.clear();
    hi.clear();
}
void PossiblePosition::addPossiblePosition(int x, int y) {
    int dx[] = {1,-1,1,-1,1,-1,0,0};
    int dy[] = {1,-1,-1,1,0,0,1,-1};
    vis[x][y] = 1;
    
    bool flag = false;
    std::vector<std::pair<int,int> > his;
    for(int i = 0; i < 8; ++i) {
        int nx = x+dx[i], ny = y+dy[i];
        if(nx < 0 || nx >= HEIGHT || ny < 0 || ny >= WIDTH || vis[nx][ny]) continue;
        
        if(!points.count(std::make_pair(nx, ny))) {
            points.insert(std::make_pair(nx, ny));
            his.push_back(std::make_pair(nx, ny));
        }
        
    }
    if(points.count(std::make_pair(x, y))) {
        flag = true;
        points.erase(std::make_pair(x, y));
    }
    hi.push_back({his, flag});
}

void PossiblePosition::rollback(int x, int y) {
    vis[x][y] = 0;
    History h = hi[hi.size()-1];
    hi.pop_back();
    for(auto& p: h.history) {
        points.erase(p);
    }
    if(h.flag)
        points.insert(std::make_pair(x,y));
}

void PossiblePosition::updatePosition(int x, int y) {
    if(points.count(std::make_pair(x, y)))
        points.erase(std::make_pair(x, y));
}
