#include <queue>
#include <iostream>

#define IN_SIZE 7

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
    void handleInput();
};
