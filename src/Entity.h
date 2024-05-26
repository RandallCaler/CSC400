#pragma once

#ifndef ENTITY_H
#define ENTITY_H

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Components.h"
#include "MatrixStack.h"
#include "Collider.h"
#include "Model.h"

#define GRAVITY -18.0f
#define AIR_RESISTANCE -14.0f
#define EPSILON 0.0001f
#define SLOPE_TOLERANCE 10

// to avoid making a separate player class, I added a few extra variables to the motion struct for the player specifically
// however, I think they could be useful for obstacle movement as well - Claire 
struct motion {
    // the velocity of the obstacles will be a constant speed in the forward direction
    glm::vec3 velocity;
    // a vector to inform which direction the object is facing
    glm::vec4 forward;

    float curSpeed;
    float curTurnSpeed;
    float upwardSpeed;
};

class Entity {
public:
    Entity();

    void updateMotion(float deltaTime, shared_ptr<Texture> hmap, vector<shared_ptr<Entity>>& collisionList);

    glm::mat4 generateModel();

    static int NEXT_ID; // initializes to 0 and increments with every call to initEntity()
    int id;
    color editorColor;
    Collider* collider = NULL;
    glm::vec3 position = glm::vec3(0);
    glm::vec3 scaleVec = glm::vec3(1);
    // float scale;
    motion m;
    float rotX;
    float rotY;
    float rotZ;
    bool grounded = false;
    bool gliding = false;
    bool collidable;
    string defaultShaderName;
    glm::mat4 modelMatrix;
    string tag;
    shared_ptr<Model> model;

};

#endif

