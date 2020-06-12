//
//  landing.cpp
//
//  This class will be used to help us determine if our lander will be in our desired landing areas or not
//
//  Project3_Final
//
//  Created by Kenny Lazaro on 5/15/20.
//

#include <stdio.h>
#include "landing.h"

bool Landing::checkInside(const ofVec3f& pos) {
    if (pos.x > parameters[0] && pos.x < parameters[1]
        && pos.z > parameters[2] && pos.z < parameters[3]) {
        landed = true;
        return true;
    }
    else return false;
}


