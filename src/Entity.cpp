
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"
#include "Shape.h"
#include "Collider.h"

using namespace std;
using namespace glm;


Entity::Entity(){};

Entity::Entity(string const& path) {
    fname = path;
};

void Entity::initEntity(std::vector<std::shared_ptr<Shape>> ref){
    objs = ref;
    for (int i = 0; i < ref.size(); i++) {
        materials m;
        material.push_back(m);
    }
    id = NEXT_ID;
   // cout << "entity created with id " << id << endl;
    NEXT_ID++;
   // cout << "NEXTID is now " << NEXT_ID << endl;
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

        float distance = sqrt((position.x * position.x) + (position.y * position.y) + (position.z * position.z));
        if(distance >= 19.5){
            m.velocity *= -1;
        }
        
        position += m.velocity * vec3(normalize(m.forward)) * deltaTime;
    
       // std::cout << "deltaTime: " << deltaTime << "entity position:" << position.x << ", " << position.y << ", " << position.z << std::endl;
        
        // TODO add collision component

}


