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
        unsigned char p0 = texData[3*((int)pixelSpaceZ * texDim.first + (int)pixelSpaceX)];
        // printf("height %i %i: %u -> %.2f\n", (int)pixelSpaceX, (int)pixelSpaceZ, texData[3*((int)pixelSpaceZ * texDim.first + (int)pixelSpaceX)], (float)(texData[3*((int)pixelSpaceZ * texDim.first + (int)pixelSpaceX)]) * scale.y / UCHAR_MAX);
        return ((float)p0 / UCHAR_MAX - 0.5) * ground.scale.y + ground.origin.y + owner->scale;
    }
    return -1;
}

float distanceOnSeparationAxis(glm::vec3 T, glm::vec3 L, glm::vec3 dimA, glm::vec3 dimB, glm::mat4 rotA, glm::mat4 rotB) {
    return std::abs(glm::dot(glm::vec3(rotA * glm::vec4(dimA.x,0,0,1)), L)) + 
        std::abs(glm::dot(glm::vec3(rotA * glm::vec4(0,dimA.y,0,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotA * glm::vec4(0,0,dimA.z,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(dimB.x,0,0,1)), L)) + 
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(0,dimB.y,0,1)), L)) +
        std::abs(glm::dot(glm::vec3(rotB * glm::vec4(0,0,dimB.z,1)), L));
}

bool Collider::isColliding(std::shared_ptr<Entity> other) {
    int i = 0;

    glm::vec3 T = other->position - owner->position;

  	mat4 ARotX = glm::rotate( glm::mat4(1.0f), owner->rotX, vec3(1, 0, 0));
  	mat4 ARotY = glm::rotate( glm::mat4(1.0f), owner->rotY, vec3(0, 1, 0));
	mat4 ARotZ = glm::rotate( glm::mat4(1.0f), owner->rotZ, vec3(0, 0, 1));
  	mat4 BRotX = glm::rotate( glm::mat4(1.0f), other->rotX, vec3(1, 0, 0));
  	mat4 BRotY = glm::rotate( glm::mat4(1.0f), other->rotY, vec3(0, 1, 0));
	mat4 BRotZ = glm::rotate( glm::mat4(1.0f), other->rotZ, vec3(0, 0, 1));
    mat4 ARot = ARotZ * ARotY * ARotX * glm::mat4(1.0f);
    mat4 BRot = BRotZ * BRotY * BRotX * glm::mat4(1.0f);
    
    glm::vec3 Ax = glm::vec3(ARot * glm::vec4(1,0,0,1));
    glm::vec3 Ay = glm::vec3(ARot * glm::vec4(0,1,0,1));
    glm::vec3 Az = glm::vec3(ARot * glm::vec4(0,0,1,1));
    glm::vec3 Bx = glm::vec3(BRot * glm::vec4(1,0,0,1));
    glm::vec3 By = glm::vec3(BRot * glm::vec4(0,1,0,1));
    glm::vec3 Bz = glm::vec3(BRot * glm::vec4(0,0,1,1));

    glm::vec3 sv1 = owner->scaleVec * 
        vec3(1.0/std::max(std::max(owner->maxBB.x - owner->minBB.x, 
            owner->maxBB.y - owner->minBB.y), 
            owner->maxBB.z - owner->minBB.z));
	glm::mat4 s1 = glm::scale(glm::mat4(1.f), sv1);

    glm::vec3 sv2 = other->scaleVec * 
        vec3(1.0/std::max(std::max(other->maxBB.x - other->minBB.x, 
            other->maxBB.y - other->minBB.y), 
            other->maxBB.z - other->minBB.z));
	glm::mat4 s2 = glm::scale(glm::mat4(1.f), sv2);

    glm::vec3 L = Ax;
    
    while (distanceOnSeparationAxis(T, L, sv1, sv2, ARot, BRot) < glm::dot(T, L) - EPSILON) {
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
                L = glm::cross(Ax, Bx);
                break;
            case 6:
                L = glm::cross(Ax, By);
                break;
            case 7:
                L = glm::cross(Ax, Bz);
                break;
            case 8:
                L = glm::cross(Ay, Bx);
                break;
            case 9:
                L = glm::cross(Ay, By);
                break;
            case 10:
                L = glm::cross(Ay, Bz);
                break;
            case 11:
                L = glm::cross(Az, Bx);
                break;
            case 12:
                L = glm::cross(Az, By);
                break;
            case 13:
                L = glm::cross(Az, Bz);
                break;
            default:
                return true;
        }
        i++;
    }
    printf("case: %2u dist: %.2f\t margin: %.2f\t\r", i, distanceOnSeparationAxis(T, L, owner->maxBB, other->maxBB, ARot, BRot), glm::dot(T, L));
    
    return false;
}

int Collider::CheckCollision(std::vector<std::shared_ptr<Entity>>& entities)
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

            if (isColliding(e)) {
                colliding = true;
                printf("collision\n");
                return i;
            }
            else {
                colliding = false;
            }
        }
    }
    return -1;
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
