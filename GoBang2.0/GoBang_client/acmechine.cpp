//
//  acmechine.cpp
//  AiBang
//
//  Created by Linkyrie on 2019/10/30.
//  Copyright © 2019 林恺锐. All rights reserved.
//

#include "acmechine.hpp"

AcMechineNode::AcMechineNode() {
    ed = false; score = 0;
    nxt[0] = nxt[1] = nxt[2] = nullptr;
}
