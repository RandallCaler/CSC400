#include "Collider.h"
#include "Entity.h"


Collider::Collider(){};

Collider::Collider(Entity *owner) : worldMin(owner->minBB), worldMax(owner->maxBB)
{
    this->owner = owner;
}

//void Collider::CheckCollision(std::vector<Entity> entities, int thisID)
//{
//    for(int i = 0; i < entities.size(); i++){
//            float distance = std::sqrt(
//            (entities[i].position.x - entities[this->entityId].position.x) * (entities[i].position.x - entities[this->entityId].position.x) + 
//            (entities[i].position.z - entities[this->entityId].position.z) * (entities[i].position.z - entities[this->entityId].position.z)
//            );
//            distance = std::abs(distance);
//            // cout << "distance is " << distance << endl;
//            // cout << "radial of other is " << entities[i].collider->GetRadial() << "compared to this radial which is " << entities[thisID].collider->GetRadial() << endl;
//            
//            if(distance < entities[i].collider->GetRadial() + entities[this->entityId].collider->GetRadial()){
//                // update this to account for butterfly collection
//                colliding = true;
//                return;
//            }
//            else {
//                colliding = false;
//            }
//        //} 
//    }
//}

float Collider::CheckGroundCollision(std::shared_ptr<Texture> hMap) {
    // translate world position of entity to pixel space of heightmap
    std::pair<int, int> texDim = hMap->getDim();
    float pixelSpaceX = (owner->position.x - ground.origin.x + texDim.first / 2) / ground.scale.x;
    float pixelSpaceZ = (owner->position.z - ground.origin.z + texDim.second / 2) / ground.scale.z;

    unsigned char* texData = hMap->getData();
    if (pixelSpaceX >= 0 && pixelSpaceX <= texDim.first && pixelSpaceZ >= 0 && pixelSpaceZ <= texDim.second) {

        int index = 3 * ((int)pixelSpaceZ * texDim.first + (int)pixelSpaceX);

        float r = (float)texData[index];
        float g = (float)texData[index + 1];
        float b = (float)texData[index + 2];
        
        float p0 = (r + g + b) / 3;
        
        return (p0 / UCHAR_MAX - 0.5) * ground.scale.y + ground.origin.y;
    }
    return -1;
}

float Collider::distanceOnSeparationAxis(glm::vec3 T, glm::vec3 L, glm::vec3 dimA, glm::vec3 dimB, glm::mat4 rotA, glm::mat4 rotB) {
    return std::abs(glm::dot(glm::vec3(rotA * glm::vec4(dimA.x,0,0,1)), L)) + 
        std::abs(glm::dot(glm::vec3(rotA * glm::vec4(0,dimA.y,0,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotA * glm::vec4(0,0,dimA.z,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(dimB.x,0,0,1)), L)) + 
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(0,dimB.y,0,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(0,0,dimB.z,1)), L));
}

glm::vec4 Collider::checkOpposingPlanes(glm::vec3 normal, glm::vec3 pointP, glm::vec3 pointN) {
    glm::vec4 planeP = glm::vec4(normal.x, normal.y, normal.z, glm::dot(normal, pointP));
    float distanceP = glm::dot(planeP, glm::vec4(owner->position, -1.0));
    glm::vec4 planeN = glm::vec4(-normal.x, -normal.y, -normal.z, glm::dot(-normal, pointN));
    float distanceN = glm::dot(planeN, glm::vec4(owner->position, -1.0));

    if (distanceP > 0 && distanceN < 0) {
        return planeP;
    }
    if (distanceN > 0 && distanceP < 0) {
        return planeN;
    }
    
    return glm::vec4(0);
}

glm::vec4 Collider::getCollisionPlane(glm::vec3 bbScale, glm::mat4 rot, std::shared_ptr<Entity> other) {
    glm::vec3 Nx = glm::vec3(rot * glm::vec4(1,0,0,1));
    glm::vec3 Ny = glm::vec3(rot * glm::vec4(0,1,0,1));
    glm::vec3 Nz = glm::vec3(rot * glm::vec4(0,0,1,1));

    glm::vec3 Ppx = other->position + glm::vec3(bbScale.x) * Nx;
    glm::vec3 Ppy = other->position + glm::vec3(bbScale.y) * Ny;
    glm::vec3 Ppz = other->position + glm::vec3(bbScale.z) * Nz;
    glm::vec3 Pnx = other->position - glm::vec3(bbScale.x) * Nx;
    glm::vec3 Pny = other->position - glm::vec3(bbScale.y) * Ny;
    glm::vec3 Pnz = other->position - glm::vec3(bbScale.z) * Nz;

    glm::vec4 normOut = vec4(0);
    normOut += checkOpposingPlanes(Nx, Ppx, Pnx);
    normOut += checkOpposingPlanes(Ny, Ppy, Pny);
    normOut += checkOpposingPlanes(Nz, Ppz, Pnz);
    return normOut;
}

glm::vec4 Collider::orientedCollision(float deltaTime, std::shared_ptr<Entity> other) {
    int i = 0;
    float distance = owner->m.curSpeed * deltaTime;

    glm::vec3 ownerVel = glm::vec3(distance * sin(owner->rotY), (owner->m.upwardSpeed + GRAVITY * deltaTime) * deltaTime, distance * cos(owner->rotY));
    glm::vec3 T = other->position - (owner->position + owner->m.velocity);
    // if(owner->id == 0) {
    //     printf("bunny xyz: %.2f %.2f %.2f cube xyz: %.2f %.2f %.2f \n", owner->position.x, owner->position.y, owner->position.z, other->position.x, other->position.y, other->position.z);
    // }

  	mat4 ARotX = glm::rotate( glm::mat4(1.0f), owner->rotX, vec3(1, 0, 0));
  	mat4 ARotY = glm::rotate( glm::mat4(1.0f), owner->rotY, vec3(0, 1, 0));
	mat4 ARotZ = glm::rotate( glm::mat4(1.0f), owner->rotZ, vec3(0, 0, 1));
  	mat4 BRotX = glm::rotate( glm::mat4(1.0f), other->rotX, vec3(1, 0, 0));
  	mat4 BRotY = glm::rotate( glm::mat4(1.0f), other->rotY, vec3(0, 1, 0));
	mat4 BRotZ = glm::rotate( glm::mat4(1.0f), other->rotZ, vec3(0, 0, 1));
    mat4 ARot = ARotX * ARotY * ARotZ * glm::mat4(1.0f);
    mat4 BRot = BRotX * BRotY * BRotZ * glm::mat4(1.0f);

    glm::vec3 Ax = glm::vec3(ARot * glm::vec4(1,0,0,1));
    glm::vec3 Ay = glm::vec3(ARot * glm::vec4(0,1,0,1));
    glm::vec3 Az = glm::vec3(ARot * glm::vec4(0,0,1,1));
    glm::vec3 Bx = glm::vec3(BRot * glm::vec4(1,0,0,1));
    glm::vec3 By = glm::vec3(BRot * glm::vec4(0,1,0,1));
    glm::vec3 Bz = glm::vec3(BRot * glm::vec4(0,0,1,1));

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
                if (Ax != Bx) {
                    L = glm::cross(Ax, Bx);
                    break;
                }
                i++;
            case 6:
                if (Ax != By) {
                    L = glm::cross(Ax, By);
                    break;
                }
                i++;
            case 7:
                if (Ax != Bz) {
                    L = glm::cross(Ax, Bz);
                    break;
                }
                i++;
            case 8:
                if (Ay != Bx) {
                   L = glm::cross(Ay, Bx);
                    break;
                }
                i++;
            case 9:
                if (Ay != By) {
                    L = glm::cross(Ay, By);
                    break;
                }
                i++;
            case 10:
                if (Ay != Bz) {
                    L = glm::cross(Ay, Bz);
                    break;
                }
                i++;
            case 11:
                if (Az != Bx) {
                    L = glm::cross(Az, Bx);
                    break;
                }
                i++;
            case 12:
                if (Az != By) {
                    L = glm::cross(Az, By);
                    break;
                }
                i++;
            case 13:
                if (Az != Bz) {
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
    for(int i = 0; i < entities.size(); i++){
        // cout << "this id = " << cat->id << " and checking id " << entities[i].id << endl;
        // /*cout << "this id = " << thisID << " and checking entities pos " << i << endl;*/
        // //if(entities[i].id != this->entityId) // exclude self when checking collisions
        // //{
        //     cout << "other entity name is " << entities[i].collider->entityName << endl;
            
        //     cout << "my type " << cat->collider->entityName << endl;
        //     cout << "here" << endl;
        //     /*cout << "this id = " << thisID << " and checking id " << entities[i].id << endl;
        //     cout << "this id = " << thisID << " and checking entities pos " << i << endl; */
        //     cout << "entity pos x = " << entities[i].position.x << endl;
        //     cout << "player pos x = " << cat->position.x << endl;
        //     cout << "entity pos z = " << entities[i].position.z << endl;
        //     cout << "player pos z = " << cat->position.z << endl;
            

            //float distance = std::sqrt(
            //(entities[i].position.x - cat->position.x) * (entities[i].position.x - cat->position.x) + 
            //(entities[i].position.z - cat->position.z) * (entities[i].position.z - cat->position.z)
            //);
            //distance = std::abs(distance);
            //if(distance < entities[i].collider->GetRadial() + cat->collider->GetRadial()){
            //    // update this to account for butterfly collection
            //    colliding = true;
            //    return i;
            //}
            //else {
            //    colliding = false;
            //}
        //} 

        shared_ptr<Entity> e = entities[i];
        if (entityId != e->id) {
            // bool iscolliding =
            //     (worldMax.x >= e->collider->worldMin.x &&
            //     worldMax.y >= e->collider->worldMin.y &&
            //     worldMax.z >= e->collider->worldMin.z) ||
            //     (worldMin.x >= e->collider->worldMax.x &&
            //     worldMin.y >= e->collider->worldMax.y &&
            //     worldMin.z >= e->collider->worldMax.z);
            glm::vec4 collisionPlane = orientedCollision(deltaTime, e);
            if (collisionPlane != glm::vec4(0)) {
                colliding = true;
                return collisionPlane;
            }
            else {
                colliding = false;
            }
        }
    }
    return glm::vec4(0);
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
