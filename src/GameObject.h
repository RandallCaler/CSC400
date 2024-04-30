//
// Created by Billy Woodward on 4/28/24.
//
#include "Entity.h"
#include "Collider.h"

#ifndef CAMERA_GAMEOBJECT_H
#define CAMERA_GAMEOBJECT_H


class GameObject : public Entity {

public:
    Collider* getColliderComponent() { return colliderComponent; }

protected:
    Collider *colliderComponent{};
};


#endif //CAMERA_GAMEOBJECT_H
