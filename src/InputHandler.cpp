#include "InputHandler.h"

#define WALK_SPEED 7.0
#define TURN_SPEED 25.0
#define JUMP_HEIGHT 8.0

InputHandler::InputHandler(){
    for (int i = 0; i < IN_SIZE; i++) {
        inputStates[i] = 0;
    }
    camRot = 0;
};

InputHandler::~InputHandler(){
};

// trying method in ex video

void InputHandler::handleInput(Entity *penguin, Camera *cam, float deltaTime) {

    float distance;
    penguin->m.curSpeed = 0.0;
    std::queue<int> q;
    std::vector<float> angles;

    bool backwards = false;
    for (int i = 0; i < IN_SIZE; i++) {
        if(inputStates[i] == 1){
            q.push(i);
            std::cout << "key pressed: " << i << std::endl;
        }
    }


    // must be tested with space bar and diagonal motion
    while(q.size() > 3) {
        std::cout << "key discarded: " << q.front() << std::endl;
        q.pop();
    }

    for (int i = 0; i < q.size(); i++) {

        int x = q.front();
        q.pop();
        q.push(x);

        switch (x) {
            case 0:
                //north
                angles.push_back(0);
                break;
            case 1:
                //west
                angles.push_back(1.57);
                break;
            case 2:
                //south
                angles.push_back(0);
                backwards = true;
                break;
            case 3:
                //east
                angles.push_back(-1.57);
                break;
            case 4:
                 //jump
                if (penguin->grounded) {
                    penguin->m.upwardSpeed = JUMP_HEIGHT;
                    penguin->grounded = false;
                }
                break;
        }
    }

    float sum = 0;

    cout << "start" << endl;
    for (int i = 0; i < angles.size(); i++) {
        sum += angles[i];
        cout << angles[i] << endl;
        penguin->m.curSpeed = WALK_SPEED;
    }
    cout << "end" << endl;

    if (backwards) {
        penguin->m.curSpeed = -WALK_SPEED;
        sum = -1 * sum;
    }

    if (angles.size() > 0) {
        penguin->rotY = sum / angles.size();
        cout << sum / angles.size() << endl;
    }
    else {
        penguin->rotY = 0;
    }
    angles.clear();
    penguin->rotY += camRot;
}


void InputHandler::setRotation(Entity *penguin, float inc) {
    camRot += inc;
    penguin->rotY = camRot;
}

