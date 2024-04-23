
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"
#include "Shape.h"
#include "Collider.h"

using namespace std;
using namespace glm;


Entity::Entity(){};

void Entity::initEntity(std::vector<std::shared_ptr<Shape>> ref){
    objs = ref;
    minBB = vec3(std::numeric_limits<float>::max());
    maxBB = vec3(std::numeric_limits<float>::min());

    for (int i = 0; i < ref.size(); i++) {
        materials m;
        material.push_back(m);

        if (minBB.x > ref[i]->min.x) minBB.x = ref[i]->min.x;
        if (minBB.y > ref[i]->min.y) minBB.y = ref[i]->min.y;
        if (minBB.z > ref[i]->min.z) minBB.z = ref[i]->min.z;
        if (maxBB.x < ref[i]->max.x) maxBB.x = ref[i]->max.x;
        if (maxBB.y < ref[i]->max.y) maxBB.y = ref[i]->max.y;
        if (maxBB.z < ref[i]->max.z) maxBB.z = ref[i]->max.z;
    }

    id = NEXT_ID;
//    cout << "entity created with id " << id << endl;
    NEXT_ID++;
//    cout << "NEXTID is now " << NEXT_ID << endl;
}

void Entity::setMaterials(int i, float r1, float g1, float b1, float r2, float g2, float b2, 
    float r3, float g3, float b3, float s) {
        material[i].matAmb.r = r1;
        material[i].matAmb.g = g1;
        material[i].matAmb.b = b1;
        material[i].matDif.r = r2;
        material[i].matDif.g = g2;
        material[i].matDif.b = b2;
        material[i].matSpec.r = r3;
        material[i].matSpec.g = g3;
        material[i].matSpec.b = b3;
        material[i].matShine = s;
}

void Entity::updateScale(float newScale){
    scale = newScale;
}

// TODO use our "game data structure" to manage all entity updates

// velocity upon collision (bounds of world, or obstacle) and will rotate the model/"flip" the forward vector
void Entity::updateMotion(float deltaTime) {
        // Calculate dot product between forward vector and velocity
        float dotProduct = glm::dot(glm::normalize(m.forward), m.velocity);

        float distance = sqrt((position.x * position.x) + (position.y * position.y) + (position.z * position.z));
        if(distance >= 19.5){
            m.velocity *= -1;
        }
        
        // Update position based on dot product and velocity
        // position += dotProduct * m.forward * deltaTime;
        position += m.velocity * deltaTime;
    
//        std::cout << "deltaTime: " << deltaTime << "entity position:" << position.x << ", " << position.y << ", " << position.z << std::endl;
        
        // TODO add collision component

}


