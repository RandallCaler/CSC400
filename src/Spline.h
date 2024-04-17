/*
* From D. McGirr (into the CG-verse previz 2019)
* This is a spline implementation that uses low order Bezier 
* curves (one or two control points) to create a path for objects
* or cameras to follow. Specify start and finish points, control
* point(s), and a duration of time that you want the path to take,
* in seconds.
* Call update(deltaTime) with the amount of time that has elapsed since the 
* last update. Call getPosition() to get the currently calculated 
* position along the spline path. Call isDone() to see when the path 
* has been completed. 
* 
*/
#ifndef SPLINE_H
#define SPLINE_H

#include "Bezier.h"

#define ORDER2 2
#define ORDER3 3

class Spline
{
private:
    float t;
    int order;
    bool done;
    float duration;
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 control1; 
    glm::vec3 control2;

public:
    Spline() {}

    // order 2 Bezier spline
    Spline(glm::vec3 start, 
           glm::vec3 control, 
           glm::vec3 end,
           float duration);    
    
    // order 3 Bezier spline
    Spline(glm::vec3 start, 
           glm::vec3 control1,
           glm::vec3 control2, 
           glm::vec3 end,
           float duration);

    ~Spline();
    void update(float deltaTime);
    bool isDone();
    glm::vec3 getPosition();
};

#endif // SPLINE_H