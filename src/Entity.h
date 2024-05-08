#pragma once

#ifndef ENTITY_H
#define ENTITY_H

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Components.h"
#include "MatrixStack.h"

#define GRAVITY -18.0
#define EPSILON 0.0001
#define SLOPE_TOLERANCE 0.5

typedef struct color {
    float r;
    float g;
    float b;
} color;


typedef struct material {
    color amb;
    color dif;
    color spec;
    float shine;
} material;

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

class Collider; // forward declaration to enable use of this class in Collider class

class Entity {
public:
    Entity();
        
    void initEntity(std::vector<std::shared_ptr<Shape>> shapes, std::vector<std::shared_ptr<Texture>> textures);

    void setMaterials(int i, float r1, float g1, float b1, float r2, float g2, float b2, 
        float r3, float g3, float b3, float s);
    void setMaterials(int i, material& mat);

    void updateMotion(float deltaTime, shared_ptr<Texture> hmap);
    void updateScale(float newScale);

  
    glm::mat4 generateModel();

    static int NEXT_ID; // initializes to 0 and increments with every call to initEntity()
    int id;
    std::vector<std::shared_ptr<Shape>> objs;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<material> materials;
    Collider* collider = NULL;
    glm::vec3 position = glm::vec3(0);
    glm::vec3 scaleVec = glm::vec3(1);
    float scale;
    motion m;
    float rotX;
    float rotY;
    float rotZ;
    bool grounded = false;
    bool gliding = false;
    string defaultShaderName;
    glm::mat4 modelMatrix;

    glm::vec3 minBB;
    glm::vec3 maxBB;
};

#endif


