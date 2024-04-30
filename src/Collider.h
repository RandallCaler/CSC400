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
    int Collider::CheckCollision(std::vector<Entity>& entities);
    void SetEntityID(int ID);
    bool IsColliding();
    void ExitCollision();
    float GetRadial();

    // AABB
    void CalculateBoundingBox(glm::mat4 modelMatrix);

    glm::vec3 worldMin;
    glm::vec3 worldMax;

private:

    int entityId;
    float radial;
    bool colliding = false;

};