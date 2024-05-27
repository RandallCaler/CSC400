
#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Program.h"
#include "Entity.h"

class Camera : public Entity
{
    public:
        Camera(glm::vec3 v, float p, float d, float a, glm::vec3 pp, float pr, glm::vec3 g, bool free = false);
        ~Camera();
        void updateCamera(float deltaTime);
        void SetView(std::shared_ptr<Program> shader, std::shared_ptr<Texture> hMap);
        // void updateCamera();
        glm::vec3 cameraPos;
        glm::vec3 lookAtPt;
        glm::vec3 upV;
        glm::vec3 view;
        glm::vec3 player_pos;
        glm::vec3 g_eye;
        glm::vec3 vel;
        float player_rot;
        float pitch;
        float dist;
        float angle;
        float horiz;
        float vert;
        float offX;
        float offZ;
        bool freeCam;

        // float pitch;
        // float yaw;
};

#endif