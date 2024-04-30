#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "Entity.h"
#include "Camera.h"
#include <queue>

#define IN_SIZE 7

// #include <iostream>


// #include <glm/gtc/type_ptr.hpp>
// #include <glm/gtc/matrix_transform.hpp>


class InputHandler
{
public:
    // the queue takes in the corresponding index to the key pressed
    std::queue<int> q;
    int inputStates[IN_SIZE];
    InputHandler();
    ~InputHandler();
    // void handleMotionPlayer();
    // void handleMotionCamera();
    void handleInput(Entity *penguin, Camera *cam);
};

#endif