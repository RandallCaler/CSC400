#ifndef ENTITY_H
#define ENTITY_H

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Components.h"

typedef struct color_vals{
    float r;
    float g;
    float b;
} color_vals;


typedef struct materials {
    color_vals matAmb;
    color_vals matDif;
    color_vals matSpec;
    float matShine;
} materials;


struct motion {
    // the velocity of the obstacles will be a constant speed in the forward direction
    float velocity;
    // a vector to inform which direction the object is facing
    glm::vec4 forward; 
};

class Collider; // forward declaration to enable use of this class in Collider class

class Entity {
    public:
    
        Entity();
        
        void initEntity(std::vector<std::shared_ptr<Shape>> ref);

        void setMaterials(int i, float r1, float g1, float b1, float r2, float g2, float b2, 
            float r3, float g3, float b3, float s);

        void updateMotion(float deltaTime);

        void updateScale(float newScale);

        static int NEXT_ID; // initializes to 0 and increments with every call to initEntity()
        int id;
        std::vector<std::shared_ptr<Shape>> objs;
        std::vector<materials> material;
        Collider* collider;
        glm::vec3 position;
        float scale;
        motion m;
        float rotate;
 
    };


#endif


