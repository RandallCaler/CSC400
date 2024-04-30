
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

void Entity::initEntity(std::vector<std::shared_ptr<Shape>> shapes, std::vector<std::shared_ptr<Texture>> textures){
    objs = shapes;
    this->textures = textures;
    minBounds = vec3(INFINITY);
    maxBounds = vec3(-INFINITY);
    for (int i = 0; i < shapes.size(); i++) {
        materials m;
        material.push_back(m);
        // build bounding box
        if (minBounds.x > objs[i]->min.x)
            minBounds.x = objs[i]->min.x;
        if (minBounds.y > objs[i]->min.y)
            minBounds.y = objs[i]->min.y;
        if (minBounds.z > objs[i]->min.z)
            minBounds.z = objs[i]->min.z;
        if (maxBounds.x < objs[i]->max.x)
            maxBounds.x = objs[i]->max.x;
        if (maxBounds.y < objs[i]->max.y)
            maxBounds.y = objs[i]->max.y;
        if (maxBounds.z < objs[i]->max.z)
            maxBounds.z = objs[i]->max.z;
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

glm::mat4 Entity::generateModel() {
    MatrixStack* M = new MatrixStack();
	M->pushMatrix();
    M->loadIdentity();
	// move to parent socket / world location 
	M->translate(position);

	// rotate about origin
	if (abs(rotX) >= EPSILON)
		M->rotate(rotX, vec3(1,0,0));
	if (abs(rotY) >= EPSILON)
		M->rotate(rotY, vec3(0,1,0));
	if (abs(rotZ) >= EPSILON)
		M->rotate(rotZ, vec3(0,0,1));

	// move object to origin and scale to a standard size, then scale to specifications
    M->scale(scaleVec * 
        vec3(1.0/std::max(std::max(maxBounds.x - minBounds.x, 
            maxBounds.y - minBounds.y), 
            maxBounds.z - minBounds.z)));
    M->translate(-vec3(0.5)*(minBounds + maxBounds));
	
	modelMatrix = M->topMatrix();
	M->popMatrix();

	return modelMatrix;
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


