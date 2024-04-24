#ifndef MANCHOT_H
#define MANCHOT_H

#include "Entity.h"

class Manchot : public Entity
{
private:
    /* data */
public:
    Manchot(/* args */);
    ~Manchot();

    void moveN();
    void moveE();
    void moveW();
    void moveS();

    void moveNE();
    void moveNW();
    void moveSE();
    void moveSW();

    void jump();
};

#endif
