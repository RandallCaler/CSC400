#ifndef MANCHOT_H
#define MANCHOT_H

#define RUN_SPEED 20
#define TURN_SPEED 160

#include "PhysicalObject.h"

struct playerMotion{
    float curSpeed;
    float curTurnSpeed;
}

class Manchot : public PhysicalObject {
    Manchot();
    handleInputs();
}


#endif // MANCHOT_H