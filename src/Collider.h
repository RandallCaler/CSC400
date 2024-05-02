#pragma once

#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED

#include "Texture.h"
#include "Shape.h"

class Entity; // forward declaration to enable use of this class in Entity class

class Collider
{


public:

    enum entityType {
        FLOWER,
        TREE,
        BUTTERFLY,
        CAT
    };
    
    char entityName;

    Collider();
    Collider(Entity *owner);
    void UpdateColliderSize();
    float CheckGroundCollision(std::shared_ptr<Texture> hMap, glm::vec3 hMapOrigin, glm::vec3 scale);
    int CheckCollision(std::vector<Entity>& entities);
    void SetEntityID(int ID);
    bool IsColliding();
    void ExitCollision();
    float GetRadial();

    // AABB
    void CalculateBoundingBox(glm::mat4 modelMatrix);

    glm::vec3 worldMin;
    glm::vec3 worldMax;
    int entityId;

private:
    int entityId;
    Entity* owner;
    float radial;
    bool colliding = false;

};

#endif