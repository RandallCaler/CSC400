#include "Manchot.h"
#include "Camera.h"

Manchot::Manchot(){
    motion.forward = vec4(0, 0, 0.1, 1);
    motion.curSpeed = 0.0;
    motion.curTurnSpeed = 0.0;
}

void Manchot::handleInputs(Camera *cam, float deltaTime){
    if (inputStates[0] == 1){
        // w
        motion.curSpeed = 3.0;
        cam->player_pos = position;

    } else if(inputStates[2] == 1){
        // s
        motion.curSpeed = -3.0;
    } else{
        motion.curSpeed = 0.0;
    }

    if (inputStates[3] == 1){
        // d
        motion.curTurnSpeed = -20.0;
    } else if (inputStates[1] == 1){
        // a
        motion.curTurnSpeed = 20.0;
    } else{
        motion.curTurnSpeed = 0.0;
    }
}
