#ifndef CAMERA_H
#define CAMERA_H

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Program.h"

class Camera
{
    public:
        Camera(glm::vec3 v, float p, float d, float a, glm::vec3 pp, float pr, glm::vec3 g);
        ~Camera();
        void SetView(std::shared_ptr<Program> shader);
        // void updateCamera();
        glm::vec3 cameraPos;
        glm::vec3 lookAtPt;
        glm::vec3 upV;
        glm::vec3 view;
        glm::vec3 player_pos;
        glm::vec3 g_eye;
        float player_rot;
        float pitch;
        float dist;
        float angle;
        float horiz;
        float vert;
        float offX;
        float offZ;

        // float pitch;
        // float yaw;
};


#endif