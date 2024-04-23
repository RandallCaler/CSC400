#include "Spline.h"

Spline::Spline(glm::vec3 start, glm::vec3 control, glm::vec3 end, float duration)
{
    t = 0;
    order = ORDER2;
    done = false;
    this->start = start;
    this->end = end;
    this->control1 = control;
    this->duration = duration;
}

Spline::Spline(glm::vec3 start, glm::vec3 control1, glm::vec3 control2, glm::vec3 end, float duration) 
{
    t = 0;
    order = ORDER3;
    done = false;
    this->start = start;
    this->end = end;
    this->control1 = control1;
    this->control2 = control2;
    this->duration = duration;
}

Spline::~Spline()
{
}

void Spline::update(float deltaTime)
{
    // scale the deltatime by the duration
    // so that it works with the parameterized 
    // Bezier functions, which expext 0<=t<=1
    t += deltaTime / duration;
    // don't overshoot 1
    if(t > 1)
    {
        t = 1;
        done = true;
    }
}

bool Spline::isDone()
{
    return done;
}

glm::vec3 Spline::getPosition()
{
    if(order == ORDER2)
        return Bezier::quadBez(Bezier::lErp, start, end, control1, t);
    else
        return Bezier::cubeBez(Bezier::lErp, start, end, control1, control2, t);
}