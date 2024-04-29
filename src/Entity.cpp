
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"
#include "Shape.h"
#include "Collider.h"

using namespace std;
using namespace glm;

int Entity::NEXT_ID = 0;

Entity::Entity(){};
void Entity::initEntity(std::vector<std::shared_ptr<Shape>> ref){
    objs = ref;
    minBB = vec3(std::numeric_limits<float>::max());
    maxBB = vec3(std::numeric_limits<float>::min());

    for (int i = 0; i < ref.size(); i++) {
        material m;
        materials.push_back(m);

        if (minBB.x > ref[i]->min.x) minBB.x = ref[i]->min.x;
        if (minBB.y > ref[i]->min.y) minBB.y = ref[i]->min.y;
        if (minBB.z > ref[i]->min.z) minBB.z = ref[i]->min.z;
        if (maxBB.x < ref[i]->max.x) maxBB.x = ref[i]->max.x;
        if (maxBB.y < ref[i]->max.y) maxBB.y = ref[i]->max.y;
        if (maxBB.z < ref[i]->max.z) maxBB.z = ref[i]->max.z;
    }

    id = NEXT_ID++;
    shaderName = "";
}

void Entity::setMaterials(int i, float r1, float g1, float b1, float r2, float g2, float b2, 
    float r3, float g3, float b3, float s) {
        materials[i].amb.r = r1;
        materials[i].amb.g = g1;
        materials[i].amb.b = b1;
        materials[i].dif.r = r2;
        materials[i].dif.g = g2;
        materials[i].dif.b = b2;
        materials[i].spec.r = r3;
        materials[i].spec.g = g3;
        materials[i].spec.b = b3;
        materials[i].shine = s;
}

void Entity::setMaterials(int i, material& mat) {
    materials[i] = mat;
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
        
        position.x += m.velocity * normalize(m.forward).x * deltaTime;
        position.y += m.velocity * normalize(m.forward).y * deltaTime;
        position.z += m.velocity * normalize(m.forward).z * deltaTime;
    
       // std::cout << "deltaTime: " << deltaTime << "entity position:" << position.x << ", " << position.y << ", " << position.z << std::endl;
        
        // TODO add collision component

}


