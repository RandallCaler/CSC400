
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


void Entity::updateBoids(float deltaTime, shared_ptr<Texture> hmap, vector<shared_ptr<Entity>> boids, shared_ptr<Entity> player){
    
    // separation forces
    vec3 separation = vec3(0, 0, 0);
    float diff;

    // match nearby velocities
    vec3 alignment = vec3(0, 0, 0);

    int activeboids = 0;
    for (int i = 0; i < boids.size(); i++){
        if(boids[i]->collider->boided){
            diff = abs(glm::distance(boids[i]->position, position));
            if(diff < 1){
                separation -= (boids[i]->position - position);
            }
            alignment += boids[i]->m.velocity;
            activeboids++;
        }
    }

    // sample weightings that Dr. Wood provided
    // overall acceleration with all forces factored inp
    if(collider->boided == true){
        collidable = false;

        if (activeboids > 1){
            alignment /= (activeboids - 1); //avg
            alignment = (alignment - m.velocity)*vec3(.005); // only match a fraction of nearby velocities
        }
        // center of motion is set to penguin position - might alter so that it trails behind penguin a bit
        vec3 leader = (player->position - this->position);
        vec3 tangentialVec = glm::cross(leader, vec3(0, 1, 0));
        // vec3 accel = (1.5f)*separation + (0.3f)*alignment + 0.2f*(leader);
        vec3 accel = (length(leader) < 3 ? 0.15f : 0.9f)*(normalize(leader)) + 0.3f*(separation) + 0.1f * normalize(tangentialVec); //

        m.velocity += accel; 
        if (glm::length(m.velocity) > 5){
            m.velocity = (m.velocity/glm::length(m.velocity)) * 2.0f;
        }

        vec4 groundPlane = collider->CheckGroundCollision(hmap);
        float groundHeight = getHeightFromPlane(groundPlane, vec2(position.x, position.z));
        // check position within penguin radius, modify velocity
        position += (m.velocity * deltaTime);
        if(position.y < (player->position.y + 0.5)){
             position.y = player->position.y + 0.5;
        }
        
        if (glm::distance(position, player->position) < 1){
            vec4 tempV = vec4(m.velocity, 1.0);
            mat4 rotM= glm::rotate(mat4(1.0f), 2.0f, vec3(0, 1, 0));
            tempV = rotM * tempV;
            // m.velocity = vec3(tempV);
            m.velocity = -1.0f*(m.velocity);
        }
        if(glm::length(scaleVec) > .5){
            scaleVec *= 0.9f;
        }    

    }
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
    /*float entityHeight = scaleVec.y* (2.0 / std::max(std::max(model->max.x - model->min.x,
            model->max.y - model->min.y),
            model->max.z - model->min.z));*/

    // ground climbing
    bool climbable = grounded && (groundHeight - groundHeight0)/(std::max(EPSILON, length(vec2(deltaX, deltaZ)))) < SLOPE_TOLERANCE || 
        position.y > groundHeight;// + entityHeight;
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

    // FALLING physics
    if (gliding == true) {
        m.upwardSpeed = std::max(m.upwardSpeed + (GRAVITY - AIR_RESISTANCE) * deltaTime, -3.0f);
    }
    else {
        m.upwardSpeed += GRAVITY * deltaTime;
    }
    position += vec3(0.0f, m.upwardSpeed * deltaTime, 0.0f);

    if (position.y > groundHeight + 0.1) { // + entityHeight
        grounded = false;
    }

    // uses the terrain height to prevent character from indefinitely falling
    if (position.y < groundHeight + EPSILON) { // + entityHeight
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
        position.y = groundHeight + EPSILON;// + entityHeight;
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
        if (position.y < groundHeight + EPSILON) { // + entityHeight
            position = oldPosition;
        }

        if (collisionPlane.y > 0 && (sliding || collisionPlane.y + EPSILON < length(vec2(collisionPlane.x, collisionPlane.z)))) {
            m.upwardSpeed -= GRAVITY * deltaTime * (collisionPlane.y * collisionPlane.y);
        }
    }
}

