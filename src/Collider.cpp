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

        unsigned char r = texData[index];
        unsigned char g = texData[index + 1];
        unsigned char b = texData[index + 2];
        
        unsigned char p0 = (r + g + b) / (3);
        
        return ((float)p0 / UCHAR_MAX - 0.5) * ground.scale.y + ground.origin.y + owner->scale;
    }
    return -1;
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
            bool iscolliding =
                (worldMax.x >= e->collider->worldMin.x &&
                worldMax.y >= e->collider->worldMin.y &&
                worldMax.z >= e->collider->worldMin.z) ||
                (worldMin.x >= e->collider->worldMax.x &&
                worldMin.y >= e->collider->worldMax.y &&
                worldMin.z >= e->collider->worldMax.z);

            if (iscolliding) {
                colliding = true;
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
