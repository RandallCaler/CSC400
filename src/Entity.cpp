
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"
#include "Shape.h"
#include "Collider.h"

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

void Entity::updateMotion(float deltaTime, shared_ptr<Texture> hmap, glm::vec4 collisionPlane) {
    float distance = m.curSpeed * deltaTime;

    // movement and rotation
    float deltaX = distance * sin(rotY);
    float deltaZ = distance * cos(rotY);
    vec3 oldPosition = position;
    vec3 newPosition = position + vec3(deltaX, 0, deltaZ);
    vec3 groundCheckPos = newPosition + vec3((distance + scale) * sin(rotY), 0, (distance + scale) * cos(rotY));

    position = groundCheckPos;
    
    float groundHeight = collider->CheckGroundCollision(hmap);
    float distanceFromGround = groundHeight - position.y;

    bool climbable = distanceFromGround < SLOPE_TOLERANCE + EPSILON;
    if (climbable) {
        position = newPosition;
    }
    else {
        position = oldPosition;
        groundHeight = collider->CheckGroundCollision(hmap);
    }

    if (position.y > groundHeight) {
        grounded = false;
    }
    if (collisionPlane.y > -EPSILON && collisionPlane.w != 0) {
        grounded = true;
        m.upwardSpeed = 0.0;
    }

    printf("\np: %.3f, %.3f, %.3f\n", position.x, position.y, position.z);
    if (collisionPlane != vec4(0)) {
        position = oldPosition;
        // printf("collision adjustment\n");
        // printf("%.3fx + %.3fy + %.3fz = %.3f\n", collisionPlane.x, collisionPlane.y, collisionPlane.z, collisionPlane.w);
        glm::vec3 delta = glm::vec3(deltaX, (m.upwardSpeed + GRAVITY *deltaTime) * deltaTime, deltaZ);
        float fP = abs(dot(oldPosition, vec3(collisionPlane)) - collisionPlane.w) + float(EPSILON);
        // printf("fp: %.3f\n");
        // glm::vec3 rev = (oldPosition + delta) + normalize(vec3(collisionPlane)) * abs(collisionPlane.w - fP + (float)EPSILON);
        vec3 rev = delta + vec3(collisionPlane) * fP;
        position += rev;
        
        // printf("p': %.3f, %.3f, %.3f\n", position.x, position.y, position.z);
    }
    else { printf("no collision\n"); }

    // FALLING physics
    if (!grounded) {
        m.upwardSpeed += GRAVITY * deltaTime;
        position += vec3(0.0f, m.upwardSpeed * deltaTime, 0.0f);

        // uses the terrain height to prevent character from indefinitely falling, will obviously have to be updated with 
        // the height value at the corresponding location
        if (position.y < groundHeight) {
            grounded = true;
            m.upwardSpeed = 0.0;
            position.y = groundHeight;
        }
    }
    
    // TODO add collision component
}


