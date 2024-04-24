#include "InputHandler.h"

InputHandler::InputHandler(){
    for (int i = 0; i < IN_SIZE; i++) {
        inputStates[i] = 0;
    }
};

InputHandler::~InputHandler(){
};

void InputHandler::handleInput(Entity penguin){
    for (int i = 0; i < IN_SIZE; i++) {
        if(inputStates[i] == 1){
            q.push(i);
            std::cout << "key pressed: " << i << std::endl;
        }
    }

    for (int i = 0; i < q.size(); i++) {
        int x = q.front();
        q.pop();

        if(inputStates[x] == 1){
            q.push(x);
        }
    }

    while(q.size() > 3) {
        std::cout << "key discarded: " << q.front() << std::endl;
        q.pop();
    }

  
    // must translate y position for jump
    for (int i = 0; i < q.size(); i++) {
        int x = q.front();
        q.pop();
        switch (x) {
            case 0:
                ///north
                // penguin.position +=  vec3(sin(cam.player_rot) * 0.1, 0, cos(cam.player_rot) * 0.1);
                penguin.position.z += -1.0;
                
                std::cout << "input state w: " << inputStates[0] << endl;
                std::cout << "entity position:" << penguin.position.x << ", " << penguin.position.y << ", " << penguin.position.z << std::endl;
        


                // set forward vector to north
                // update entity motion
                break;
            case 1:
                //west
                penguin.position += vec3(-1, 0, 0);
                std::cout << "input state a: " << inputStates[1] << endl;
                std::cout << "entity position:" << penguin.position.x << ", " << penguin.position.y << ", " << penguin.position.z << std::endl;
        


                
                break;
            // case 2:
            //     //south
            //     break;
            // case 3:
            //     //east
            //     break;
            // case 4:
            //     //jump
            //     break;
        }
       // q.push(x);
    }

}