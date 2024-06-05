
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"
#include "Shape.h"
#include "Collider.h"

using namespace std;
using namespace glm;

int Entity::NEXT_ID = 0;

Entity::Entity() {
    m.curSpeed = 0.0;
    m.curTurnSpeed = 0.0;
    m.upwardSpeed = 0.0;
    m.forward = vec4(0, 0, -5, 0);
    rotX = 0.0;
    rotY = 0.0;
    rotZ = 0.0;
    grounded = true;
    id = NEXT_ID++;
    defaultShaderName = "";

    int r = (id * 137) % 256;  // Using a larger multiplier for more spread
    int g = (id * 149) % 256;  // Different multipliers for each color component
    int b = (id * 163) % 256;

    // set diffuse mats, converting from [0,255]i to [0,1]f
    color em = { r / 255.0f, g / 255.0f, b / 255.0f };
    editorColor = em;
};

// void Entity::updateScale(float newScale) {
//     scale = newScale;
// }

glm::mat4 Entity::generateModel() {
    MatrixStack* M = new MatrixStack();
    M->pushMatrix();
    M->loadIdentity();
    // move to parent socket / world location 
    M->translate(position);

    // rotate about origin
    if (abs(rotX) >= EPSILON)
        M->rotate(rotX, vec3(1, 0, 0));
    if (abs(rotY) >= EPSILON)
        M->rotate(rotY, vec3(0, 1, 0));
    if (abs(rotZ) >= EPSILON)
        M->rotate(rotZ, vec3(0, 0, 1));

    // move object to origin and scale to a standard size, then scale to specifications
    M->scale(scaleVec *
        vec3(1.0 / std::max(std::max(model->max.x - model->min.x,
            model->max.y - model->min.y),
            model->max.z - model->min.z)));
    M->translate(-vec3(0.5) * (model->min + model->max));

    modelMatrix = M->topMatrix();
    M->popMatrix();

    return modelMatrix;
}

float getHeightFromPlane(vec4 plane, vec2 pos) {
    return (plane.w - plane.x * pos.x - plane.z * pos.y) / plane.y;
}

void Entity::updateBoids(float deltaTime, vector<Entity> *boids, Entity *curBoid, Entity *penguin){
    vec3 aSeparation = separationForce(boids, curBoid);
    vec3 aAlignment = alignmentForce(boids, curBoid);
    vec3 aLeader = penguin->position;

    // sample weightings that Dr. Wood provided
    vec3 accel = (1.5f)*aSeparation + (0.3f)*aAlignment + 0.2f*(aLeader);
    
    curBoid->velocity += accel; 
    curBoid->position += (curBoid->velocity * deltaTime); 
}

vec3 Entity::separationForce(vector<Entity> *boids, Entity *curBoid){
    vec3 displacement = vec3(0, 0, 0);
    vec3 diff;

    for (int i = 0; i < boids.size(); i++){
        if(boids[i] != curBoid){
            diff = boids[i].position - curBoid.position;
            if(abs(diff)){
                displacement -= diff;
            }
        }
    }

    return displacement;
}

vec3 Entity::alignmentForce(vector<Entity> *boids, Entity *curBoid){
    vec3 avg = vec3(0, 0, 0);

    for (int i = 0; i < boids.size(); i++){
        if(boids[i] != curBoid)
            avg += boids[i].velocity;
    }

    avg /= (boids.size() - 1);
    return ((avg - curBoid.velocity)/8);
    
}


void Entity::updateMotion(float deltaTime, shared_ptr<Texture> hmap, vector<shared_ptr<Entity>>& collisionList, int *collisionSounds) {
    float distance = m.curSpeed * deltaTime;
    // movement and rotation
    float deltaX = distance * sin(rotY);
    float deltaZ = distance * cos(rotY);
    
    vec3 oldPosition = position;
    vec3 newPosition = position + vec3(deltaX, 0, deltaZ);

    // get ground samples
    vec4 groundPlane0 = collider->CheckGroundCollision(hmap);
    float groundHeight0 = getHeightFromPlane(groundPlane0, vec2(oldPosition.x, oldPosition.z));

    vec3 newPositionForward = newPosition + vec3((scaleVec.z/2) * sin(rotY), 0, (scaleVec.z/2) * cos(rotY));
    position = newPositionForward;
    vec4 groundPlane = collider->CheckGroundCollision(hmap);
    float groundHeight = getHeightFromPlane(groundPlane, vec2(newPositionForward.x, newPositionForward.z));

    vec3 newPositionBack = newPosition - vec3((scaleVec.z/2) * sin(rotY), 0, (scaleVec.z/2) * cos(rotY));
    position = newPositionBack;
    vec4 groundPlaneB = collider->CheckGroundCollision(hmap);
    float groundHeightB = getHeightFromPlane(groundPlaneB, vec2(newPositionBack.x, newPositionBack.z));

    if (groundHeight < groundHeightB) {
        groundPlane = groundPlaneB;
        groundHeight = groundHeightB;
    }
    float entityHeight = scaleVec.y/2;

    // ground climbing
    bool climbable = grounded && (groundHeight - groundHeight0)/(std::max(EPSILON, length(vec2(deltaX, deltaZ)))) < SLOPE_TOLERANCE || 
        position.y > groundHeight + entityHeight;

    if (climbable || length(vec2(deltaX, deltaZ)) < EPSILON) {
        position = newPosition;
    }
    // if the area of ground ahead cannot be climbed, return to previous position
    else {
        position = oldPosition;
        // sliding = true;
        groundPlane = groundPlane0;
        groundHeight = groundHeight0;
    }

    // printf("%.3f\t", m.upwardSpeed);
    // FALLING physics
    if (gliding == true) {
        m.upwardSpeed = std::max(m.upwardSpeed + (GRAVITY - AIR_RESISTANCE) * deltaTime, -3.0f);
    }
    else {
        m.upwardSpeed += GRAVITY * deltaTime;
    }
    // printf("+ %.3f * %.3f -> %.3f\t", GRAVITY, deltaTime, m.upwardSpeed);
    position += vec3(0.0f, m.upwardSpeed * deltaTime, 0.0f);

    if (position.y > groundHeight + entityHeight + 0.1) {
        grounded = false;
    }

    // uses the terrain height to prevent character from indefinitely falling
    if (position.y < groundHeight + entityHeight) {
        if (!sliding) {
            grounded = true;
        }
        // if (groundPlane.y > 0 && (sliding || collisionPlane.y + EPSILON < length(vec2(collisionPlane.x, collisionPlane.z)))) {
        else {
            m.upwardSpeed -= GRAVITY * deltaTime * (groundPlane.y * groundPlane.y);
            m.curSpeed -= (groundPlane.x * sin(rotY) + groundPlane.z * cos(rotY)) * GRAVITY * deltaTime;
            printf("%.3f\n", m.curSpeed);
        }
        gliding = false;
        m.upwardSpeed = std::max(0.0f, m.upwardSpeed);
        position.y = groundHeight + entityHeight;
        // printf("grounded: %.4f vs %.4f\n", position.y - entityHeight, groundHeight);
    }

    vec3 collisionPlane = vec3(collider->CheckCollision(deltaTime, collisionList, collisionSounds));
    // oriented bounding box restrictions
    if (collisionPlane != vec3(0)) {
        // handle gravity response when colliding with y planes
        // binary response: the player is on a standing surface and is grounded, or is not and will slide off it

        // stand on top face
        if (!sliding && collisionPlane.y + EPSILON > length(vec2(collisionPlane.x, collisionPlane.z))) {
            grounded = true;
            gliding = false;
            m.upwardSpeed = std::max(0.0f, m.upwardSpeed);
        }

        // bounce off bottom face
        if (collisionPlane.y - EPSILON < -length(vec2(collisionPlane.x, collisionPlane.z))) {
            m.upwardSpeed = std::min(0.0f, m.upwardSpeed);
        }

        // acquire new position along bounding box face
        vec3 delta = position - oldPosition;
        position = oldPosition;
        float fP = abs(dot(delta, collisionPlane));
        position += delta + collisionPlane * fP;
        if (position.y < groundHeight + entityHeight) {
            position = oldPosition;
        }

        if (collisionPlane.y > 0 && (sliding || collisionPlane.y + EPSILON < length(vec2(collisionPlane.x, collisionPlane.z)))) {
            m.upwardSpeed -= GRAVITY * deltaTime * (collisionPlane.y * collisionPlane.y);
        }
    }
}

