
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"
#include "Shape.h"
#include "Collider.h"

#define GRAVITY -10.0
#define TERRAIN_HEIGHT -0.1

using namespace std;
using namespace glm;

int Entity::NEXT_ID = 0;

Entity::Entity(){
    m.curSpeed = 0.0;
    m.curTurnSpeed = 0.0;
    m.upwardSpeed = 0.0;
    m.forward = vec4(0, 0, -5, 0);
    rotX = 0.0;
    rotY = 0.0;
    rotZ = 0.0;
    grounded = true;
};

void Entity::initEntity(std::vector<std::shared_ptr<Shape>> shapes, std::vector<std::shared_ptr<Texture>> textures){
    objs = shapes;
    this->textures = textures;
    minBB = vec3((std::numeric_limits<float>::max)());
    maxBB = vec3((std::numeric_limits<float>::min)());

    for (int i = 0; i < shapes.size(); i++) {
        material m;
        materials.push_back(m);

        if (minBB.x > shapes[i]->min.x) minBB.x = shapes[i]->min.x;
        if (minBB.y > shapes[i]->min.y) minBB.y = shapes[i]->min.y;
        if (minBB.z > shapes[i]->min.z) minBB.z = shapes[i]->min.z;
        if (maxBB.x < shapes[i]->max.x) maxBB.x = shapes[i]->max.x;
        if (maxBB.y < shapes[i]->max.y) maxBB.y = shapes[i]->max.y;
        if (maxBB.z < shapes[i]->max.z) maxBB.z = shapes[i]->max.z;
    }

    id = NEXT_ID++;
    defaultShaderName = "";
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
        vec3(1.0/std::max(std::max(maxBB.x - minBB.x, 
            maxBB.y - minBB.y), 
            maxBB.z - minBB.z)));
    M->translate(-vec3(0.5)*(minBB + maxBB));
	
	modelMatrix = M->topMatrix();
	M->popMatrix();

	return modelMatrix;
}


void Entity::updateMotion(float deltaTime) {
    // this method does not use the forward vector (simpler math)
    // if entity == player
        //rotY = m.curTurnSpeed * deltaTime;
        float distance = m.curSpeed * deltaTime;

        // movement and rotation
        float deltaX = distance * sin(rotY);
        float deltaZ = distance * cos(rotY);
        position += vec3(deltaX, 0.0f, deltaZ);

        // FALLING physics
        m.upwardSpeed += GRAVITY * deltaTime;
        position += vec3(0.0f, m.upwardSpeed * deltaTime, 0.0f);

        // uses the terrain height to prevent character from indefinitely falling, will obviously have to be updated with 
        // the height value at the corresponding location
        if (position.y < TERRAIN_HEIGHT){
            grounded = true;
            m.upwardSpeed = 0.0;
            position.y = TERRAIN_HEIGHT;
        }

        // std::cout << "position in entity " << position.x << " " << position.y << " " << position.z << endl;


        // float distance = sqrt((position.x * position.x) + (position.y * position.y) + (position.z * position.z));
        // if(distance >= 19.5){
        //     m.velocity *= -1;
        // }
        
        // position += m.velocity * vec3(normalize(m.forward)) * deltaTime;
//     float distance = sqrt((position.x * position.x) + (position.y * position.y) + (position.z * position.z));
//     if(distance >= 19.5){
//         m.velocity *= -1;
//     }
    
//     position += m.velocity * vec3(normalize(m.forward)) * deltaTime;

    // std::cout << "deltaTime: " << deltaTime << "entity position:" << position.x << ", " << position.y << ", " << position.z << std::endl;
    
    // TODO add collision component
}


