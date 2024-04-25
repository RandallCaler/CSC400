#include "InputHandler.h"

InputHandler::InputHandler(){
    for (int i = 0; i < IN_SIZE; i++) {
        inputStates[i] = 0;
    }
};

InputHandler::~InputHandler(){
};

void InputHandler::handleInput(Entity *penguin, Camera *cam){
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
        vec4 norm;
        vec4 tempF;
        float angle;
        float epsilon = 0.001;

        glm::mat4 westRotation = glm::rotate(glm::mat4(1.0f), 1.71f, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 eastRotation = glm::rotate(glm::mat4(1.0f), -1.71f, glm::vec3(0.0f, 1.0f, 0.0f));

        int x = q.front();
        q.pop();

        switch (x) {
            case 0:
                ///north
                // penguin.position +=  vec3(sin(cam.player_rot) * 0.1, 0, cos(cam.player_rot) * 0.1);
                norm = glm::normalize(penguin->m.forward);
                penguin->position += vec3(norm);
                cam->player_pos = penguin->position;

                
                std::cout << "input state w: " << inputStates[0] << endl;
                std::cout << "entity position:" << penguin->position.x << ", " << penguin->position.y << ", " << penguin->position.z << std::endl;
        

                // set forward vector to north
                // update entity motion
                break;
            case 1:
                //west

                tempF = westRotation * penguin->m.forward;
                norm = glm::normalize(tempF);

                // if angle between camera and tempf is NOT 90, set 90
                // angle = acos( dot( normalize(y-x), normalize(z-x) ) )
                angle = glm::acos(glm::dot(vec3(norm), glm::normalize(cam->player_pos - cam->cameraPos)));

                if((angle < (1.57 - epsilon)) || (angle > (1.57 + epsilon))){
                    penguin->position += vec3(norm);
                }
                else{
                    penguin->position += vec3(glm::normalize(penguin->m.forward));
                }
        
                cam->player_pos = penguin->position;

                std::cout << "input state a: " << inputStates[1] << endl;
                std::cout << "entity position:" << penguin->position.x << ", " << penguin->position.y << ", " << penguin->position.z << std::endl;
                        
                break;
            case 2:
                //south

                // must handle, if angle is currently towards A, then turn ccw, if angle is towards D, turn cw

                tempF = penguin->m.forward;
                norm = glm::normalize(tempF);

                angle = glm::acos(glm::dot(vec3(norm), glm::normalize(cam->player_pos - cam->cameraPos)));

                if ((angle > 0.0 + epsilon) && (angle > 0.0 - epsilon)) {
                    penguin->position += (vec3(norm) * vec3(-1.0, -1.0, -1.0));
                    // penguin->position += vec3(0.0, 0.0, 1.0);
                }
                else if ((angle > (1.57 - epsilon)) && angle < (1.57 + epsilon)) {
                    tempF = westRotation * tempF;
                    penguin->position += vec3(glm::normalize(tempF));
                }
                else if ((angle > (-1.57 - epsilon)) && (angle > (-1.57+ epsilon))) {
                    tempF = eastRotation * tempF;
                    penguin->position += vec3(glm::normalize(tempF));
                }

                cam->player_pos = penguin->position;

                std::cout << "input state s: " << inputStates[2] << endl;
                std::cout << "entity position:" << penguin->position.x << ", " << penguin->position.y << ", " << penguin->position.z << std::endl;
                      

                break;
            case 3:
                //east

                tempF = eastRotation * penguin->m.forward;
                norm = glm::normalize(tempF);

                angle = glm::acos(glm::dot(vec3(norm), glm::normalize(cam->player_pos - cam->cameraPos)));

                if((angle < (-1.57 - epsilon)) || (angle > (-1.57 + epsilon))){
                    penguin->position += vec3(norm);
                }
                else{
                    penguin->position += vec3(glm::normalize(penguin->m.forward));
                }

                cam->player_pos = penguin->position;

                std::cout << "input state a: " << inputStates[1] << endl;
                std::cout << "entity position:" << penguin->position.x << ", " << penguin->position.y << ", " << penguin->position.z << std::endl;
                        
            
                break;
                
            // case 4:
            //     //jump
            //     break;
        }
       // q.push(x);
    }

}