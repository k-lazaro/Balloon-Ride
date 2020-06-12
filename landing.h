
#pragma once

#include "ofMain.h"

class Landing {
public:
    Landing() {}
    Landing(const float& minX, const float& maxX, const float& minZ, const float& maxZ) {
        parameters[0] = minX;
        parameters[1] = maxX;
        parameters[2] = minZ;
        parameters[3] = maxZ;
    }

    bool checkInside(const ofVec3f& pos);

    float parameters[4];
    bool landed = false;
};
