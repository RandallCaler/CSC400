#include "Collider.h"
#include "Entity.h"

#include <math.h>


Collider::Collider(){};

Collider::Collider(Entity *owner, bool collectible) : worldMin(owner->minBB), worldMax(owner->maxBB)
{
    this->owner = owner;
    this->collectible = collectible;
}

float sampleHeightFromPixel(int x, int z, int major, unsigned char*& data) {
    int index = 3 * (z * major + x);
    float r = (float)data[index];
    float g = (float)data[index + 1];
    float b = (float)data[index + 2];
    return (r + g + b) / 3;
}

vec3 Collider::pixelToWorldSpace(vec3 p, pair<int, int> mapSize) {
    return vec3(ground.scale.x * (p.x - mapSize.first / 2) - ground.origin.x, (p.y / UCHAR_MAX - 0.5) * ground.scale.y + ground.origin.y, ground.scale.z * (p.z - mapSize.second / 2) - ground.origin.z);
}

/*
 * CheckGroundCollision - obtain the plane equation of the ground below the owner
 * takes a heightmap, outputs a vec4 where xyz describe the normal and w completes a plane equation
 */
vec4 Collider::CheckGroundCollision(std::shared_ptr<Texture> hMap) {
    // translate world position of entity to pixel space of heightmap
    std::pair<int, int> texDim = hMap->getDim();
    float pixelSpaceX = (owner->position.x - ground.origin.x) / ground.scale.x + texDim.first / 2;
    float pixelSpaceZ = (owner->position.z - ground.origin.z) / ground.scale.z + texDim.second / 2;

    unsigned char* texData = hMap->getData();

    // check whether the entity position corresponds to any valid heightmap pixel
    if (pixelSpaceX >= 0 && pixelSpaceX <= texDim.first && pixelSpaceZ >= 0 && pixelSpaceZ <= texDim.second) {
        // floor
        int pixelIndexZ = (int)pixelSpaceZ;
        int pixelIndexX = (int)pixelSpaceX;

        // rgb average of the pixel being sampled
        float heightA, heightB, heightC = 0.0f;
        heightA = sampleHeightFromPixel(pixelIndexX, pixelIndexZ, texDim.first, texData);
        heightB = sampleHeightFromPixel(pixelIndexX + 1, pixelIndexZ + 1, texDim.first, texData);
        
        // world space coordinates of each point on the triangle
        vec3 pA = pixelToWorldSpace(vec3(pixelIndexX, heightA, pixelIndexZ), texDim);
        vec3 pB = pixelToWorldSpace(vec3(pixelIndexX + 1, heightB, pixelIndexZ + 1), texDim);
        vec3 pC, normal;

        // the ground mesh is a grid where each square is divided into two triangles
        // thus, there are two different planes depending on where owner is in the square
        if ((pixelSpaceX - pixelIndexX) > (pixelSpaceZ - pixelIndexZ)) {
            heightC = sampleHeightFromPixel(pixelIndexX + 1, pixelIndexZ, texDim.first, texData);
            pC = pixelToWorldSpace(vec3(pixelIndexX + 1, heightC, pixelIndexZ), texDim);
            // cross-product the three points to obtain the normal of the plane they share
            normal = normalize(cross(pB - pA, pC - pA));
        }
        else {
            heightC = sampleHeightFromPixel(pixelIndexX, pixelIndexZ + 1, texDim.first, texData);
            pC = pixelToWorldSpace(vec3(pixelIndexX, heightC, pixelIndexZ + 1), texDim);
            normal = normalize(cross(pC - pA, pB - pA));
        }

        // w = (px)x + (py)y + (pz)z, completing plane equation
        return vec4(normal, dot(normal, pA));
    }

    return vec4(0);
}

float Collider::distanceOnSeparationAxis(glm::vec3 T, glm::vec3 L, glm::vec3 dimA, glm::vec3 dimB, glm::mat4 rotA, glm::mat4 rotB) {
    return std::abs(glm::dot(glm::vec3(rotA * glm::vec4(dimA.x,0,0,1)), L)) + 
        std::abs(glm::dot(glm::vec3(rotA * glm::vec4(0,dimA.y,0,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotA * glm::vec4(0,0,dimA.z,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(dimB.x,0,0,1)), L)) + 
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(0,dimB.y,0,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(0,0,dimB.z,1)), L));
}

bool compareVec3(vec3 a, vec3 b) {
    return abs(a.x - b.x) < EPSILON &&
        abs(a.y - b.y) < EPSILON &&
        abs(a.z - b.z) < EPSILON;
}

glm::vec4 Collider::checkOpposingPlanes(glm::vec3 normal, glm::vec3 pointP, glm::vec3 pointN) {
    // solve plane equations for opposing faces on the same principal axis
    glm::vec4 planeP = glm::vec4(normal.x, normal.y, normal.z, glm::dot(normal, pointP));
    float distanceP = glm::dot(planeP, glm::vec4(owner->position, -1.0));
    glm::vec4 planeN = glm::vec4(-normal.x, -normal.y, -normal.z, glm::dot(-normal, pointN));
    float distanceN = glm::dot(planeN, glm::vec4(owner->position, -1.0));

    // return the plane equation of the face being collided with
    if (distanceP > 0 && distanceN < 0) {
        return planeP;
    }
    if (distanceN > 0 && distanceP < 0) {
        return planeN;
    }
    // if owner is between both planes, it is not colliding on those planes
    return glm::vec4(0);
}

glm::vec4 Collider::getCollisionPlane(glm::vec3 bbScale, glm::mat4 rot, std::shared_ptr<Entity> other) {
    // normals in each principal axis of other's bounding box
    glm::vec3 Nx = glm::vec3(rot * glm::vec4(1,0,0,1));
    glm::vec3 Ny = glm::vec3(rot * glm::vec4(0,1,0,1));
    glm::vec3 Nz = glm::vec3(rot * glm::vec4(0,0,1,1));

    // points on each face of other's bounding box
    glm::vec3 Ppx = other->position + glm::vec3(bbScale.x) * Nx;
    glm::vec3 Ppy = other->position + glm::vec3(bbScale.y) * Ny;
    glm::vec3 Ppz = other->position + glm::vec3(bbScale.z) * Nz;
    glm::vec3 Pnx = other->position - glm::vec3(bbScale.x) * Nx;
    glm::vec3 Pny = other->position - glm::vec3(bbScale.y) * Ny;
    glm::vec3 Pnz = other->position - glm::vec3(bbScale.z) * Nz;

    // compile plane equations of the faces owner is colliding with
    glm::vec4 normOut = vec4(0);
    glm::vec4 x = checkOpposingPlanes(Nx, Ppx, Pnx);
    normOut += x;
    x = checkOpposingPlanes(Ny, Ppy, Pny);
    normOut += x;
    x = checkOpposingPlanes(Nz, Ppz, Pnz);
    normOut += x;
    return normOut;
}

glm::vec4 Collider::orientedCollision(float deltaTime, std::shared_ptr<Entity> other) {
    int i = 0;
    float distance = owner->m.curSpeed * deltaTime;
    glm::vec3 ownerVel = glm::vec3(distance * sin(owner->rotY), (owner->m.upwardSpeed + GRAVITY * deltaTime) * deltaTime, distance * cos(owner->rotY));
    
    // displacement vector
    glm::vec3 T = other->position - (owner->position + owner->m.velocity);

    // build each entity's rotation matrix
  	mat4 ARotX = glm::rotate( glm::mat4(1.0f), owner->rotX, vec3(1, 0, 0));
  	mat4 ARotY = glm::rotate( glm::mat4(1.0f), owner->rotY, vec3(0, 1, 0));
	mat4 ARotZ = glm::rotate( glm::mat4(1.0f), owner->rotZ, vec3(0, 0, 1));
  	mat4 BRotX = glm::rotate( glm::mat4(1.0f), other->rotX, vec3(1, 0, 0));
  	mat4 BRotY = glm::rotate( glm::mat4(1.0f), other->rotY, vec3(0, 1, 0));
	mat4 BRotZ = glm::rotate( glm::mat4(1.0f), other->rotZ, vec3(0, 0, 1));
    mat4 ARot = ARotX * ARotY * ARotZ * glm::mat4(1.0f);
    mat4 BRot = BRotX * BRotY * BRotZ * glm::mat4(1.0f);

    // unit vectors facing orthogonal to each bounding box's faces
    glm::vec3 Ax = glm::vec3(ARot * glm::vec4(1,0,0,1));
    glm::vec3 Ay = glm::vec3(ARot * glm::vec4(0,1,0,1));
    glm::vec3 Az = glm::vec3(ARot * glm::vec4(0,0,1,1));
    glm::vec3 Bx = glm::vec3(BRot * glm::vec4(1,0,0,1));
    glm::vec3 By = glm::vec3(BRot * glm::vec4(0,1,0,1));
    glm::vec3 Bz = glm::vec3(BRot * glm::vec4(0,0,1,1));

    // scale bounding boxes to world space
    float scalefactor1 = 1.0/(std::max(owner->maxBB.x - owner->minBB.x, 
            owner->maxBB.y - owner->minBB.y), 
            owner->maxBB.z - owner->minBB.z);
    glm::vec3 sv1 = owner->scaleVec * 
        glm::vec3((owner->maxBB.x - owner->minBB.x)/2*scalefactor1,
            (owner->maxBB.y - owner->minBB.y)/2*scalefactor1, 
            (owner->maxBB.z - owner->minBB.z)/2*scalefactor1);

    float scalefactor2 = 1.0/(std::max(other->maxBB.x - other->minBB.x, 
            other->maxBB.y - other->minBB.y), 
            other->maxBB.z - other->minBB.z);
    glm::vec3 sv2 = other->scaleVec * 
        glm::vec3((other->maxBB.x - other->minBB.x)/2 *scalefactor2,
            (other->maxBB.y - other->minBB.y)/2*scalefactor2, 
            (other->maxBB.z - other->minBB.z)/2*scalefactor2);

    glm::vec3 L = Ax;
    
    while (distanceOnSeparationAxis(T, L, sv1, sv2, ARot, BRot) > std::abs(glm::dot(T, L))) {
        /*
         * separation axis cases: if there is a separation between the boxes 
         * projected onto L's corresponding separation plane, there is no collision
         */
        switch (i) {
            case 0:
                L = Ay;
                break;
            case 1:
                L = Az;
                break;
            case 2:
                L = Bx;
                break;
            case 3:
                L = By;
                break;
            case 4:
                L = Bz;
                break;
            case 5:
                if (!compareVec3(Ax, Bx)) {
                    L = glm::cross(Ax, Bx);
                    break;
                }
                i++;
            case 6:
                if (!compareVec3(Ax, By)) {
                    L = glm::cross(Ax, By);
                    break;
                }
                i++;
            case 7:
                if (!compareVec3(Ax, Bz)) {
                    L = glm::cross(Ax, Bz);
                    break;
                }
                i++;
            case 8:
                if (!compareVec3(Ay, Bx)) {
                   L = glm::cross(Ay, Bx);
                    break;
                }
                i++;
            case 9:
                if (!compareVec3(Ay, By)) {
                    L = glm::cross(Ay, By);
                    break;
                }
                i++;
            case 10:
                if (!compareVec3(Ay, Bz)) {
                    L = glm::cross(Ay, Bz);
                    break;
                }
                i++;
            case 11:
                if (!compareVec3(Az, Bx)) {
                    L = glm::cross(Az, Bx);
                    break;
                }
                i++;
            case 12:
                if (!compareVec3(Az, By)) {
                    L = glm::cross(Az, By);
                    break;
                }
                i++;
            case 13:
                if (!compareVec3(Az, Bz)) {
                    L = glm::cross(Az, Bz);
                    break;
                }
                i++;
            default:
                // collision case: get the plane owner is colliding with
                return getCollisionPlane(sv2, BRot, other);
        }
        
        i++;
    }

    return glm::vec4(0);
}

glm::vec4 Collider::CheckCollision(float deltaTime, std::vector<std::shared_ptr<Entity>>& entities)
{
    glm::vec4 collisionPlane = vec4(0);
    colliding = false;

    for(int i = 0; i < entities.size(); i++) {
        shared_ptr<Entity> e = entities[i];
        // check collisions with each entity in the list except for the entity doing the checking
        if (entityId != e->id) {
            glm::vec4 newCPlane = orientedCollision(deltaTime, e);
            if (collisionPlane != vec4(0)) {
                // if the owner is colliding with multiple boxes, adjust additional plane equations to prevent face-sinking
                collisionPlane -= dot(vec3(newCPlane), vec3(collisionPlane))*newCPlane;
            }
            // get plane equation for an oriented bounding box collision
            collisionPlane += newCPlane;
            if (newCPlane != glm::vec4(0)) {
                if (e->collider->collectible) {
                    // placeholder collectible response - should activate boid behavior
                    e->position.y += 100;
                }
                else {
                    colliding = true;
                }
            }
        }
    }
    return collisionPlane;
}

void Collider::UpdateColliderSize(){

}

void Collider::SetEntityID(int ID){
    entityId = ID;
}

bool Collider::IsColliding(){
    return colliding;
}

void Collider::ExitCollision(){
    colliding = false;
}

float Collider::GetRadial(){
    return radial;
}

void Collider::CalculateBoundingBox(glm::mat4 modelMatrix) {
    glm::vec3 vertices[8] = {
    glm::vec3(worldMin.x, worldMin.y, worldMin.z),
    glm::vec3(worldMax.x, worldMin.y, worldMin.z),
    glm::vec3(worldMin.x, worldMax.y, worldMin.z),
    glm::vec3(worldMax.x, worldMax.y, worldMin.z),
    glm::vec3(worldMin.x, worldMin.y, worldMax.z),
    glm::vec3(worldMax.x, worldMin.y, worldMax.z),
    glm::vec3(worldMin.x, worldMax.y, worldMax.z),
    glm::vec3(worldMax.x, worldMax.y, worldMax.z)
    };

    // Transform vertices by the model matrix to get world coordinates
    for (int i = 0; i < 8; ++i)
    {
        vertices[i] = glm::vec3(modelMatrix * glm::vec4(vertices[i], 1.0f));
    }

    // Find the bounds
    glm::vec3 newMin = vertices[0];
    glm::vec3 newMax = vertices[0];

    for (int i = 1; i < 8; ++i)
    {
        newMin = glm::min(newMin, vertices[i]);
        newMax = glm::max(newMax, vertices[i]);
    }

    this->worldMin = newMin;
    this->worldMax = newMax;

}
