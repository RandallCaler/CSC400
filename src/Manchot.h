#include "Entity.h"

class Manchot : public Entity
{
private:
    /* data */
public:
    Manchot(/* args */);
    ~Manchot();
    void moveLeft();
    void moveRight();
    void moveForward();
    void moveBackward();
    void jump();
};

Manchot::Manchot(/* args */)
{
}

Manchot::~Manchot()
{
}
