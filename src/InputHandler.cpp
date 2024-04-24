#include "InputHandler.h"

InputHandler::InputHandler(){
    for (int i = 0; i < IN_SIZE; i++) {
        inputStates[i] = 0;
    }
};

InputHandler::~InputHandler(){
};

void InputHandler::handleInput(){
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

}