#pragma once

#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED

#include "Texture.h"
#include "Shape.h"

class Entity; // forward declaration to enable use of this class in Entity class

typedef struct groundProperties {
    glm::vec3 origin;
    glm::vec3 scale;
} Ground;

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
    float CheckGroundCollision(std::shared_ptr<Texture> hMap);
    glm::vec3 CheckCollision(float deltaTime, std::vector<std::shared_ptr<Entity>>& entities);
    void SetEntityID(int ID);
    void SetGround(glm::vec3 origin, glm::vec3 scale) { ground.origin = origin; ground.scale = scale; };
    bool IsColliding();
    float distanceOnSeparationAxis(glm::vec3 T, glm::vec3 L, glm::vec3 dimA, glm::vec3 dimB, glm::mat4 rotA, glm::mat4 rotB);
    glm::vec3 checkOpposingPlanes(glm::vec3 normal, glm::vec3 pointP, glm::vec3 pointN);
    glm::vec3 getCollisionNormal(glm::vec3 bbScale, glm::mat4 rot, std::shared_ptr<Entity> other);
    glm::vec3 orientedCollision(float deltaTime, std::shared_ptr<Entity> other);
    void ExitCollision();
    float GetRadial();

    // AABB
    void CalculateBoundingBox(glm::mat4 modelMatrix);

    glm::vec3 worldMin;
    glm::vec3 worldMax;
    Ground ground;

private:
    int entityId;
    Entity* owner;
    float radial;
    bool colliding = false;

};

#endif