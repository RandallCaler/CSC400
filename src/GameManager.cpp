#include "GameManager.h"

GameManager::GameManager(){}

void GameManager::checkDeath() {
	if (player->position.y <= deathHeight) {
		respawn();
	}
}

void GameManager::checkCollected() {
	// check if Manchot collected some picnic goodness
	for (shared_ptr<Entity> col : collectibles) {
		if (glm::distance(player->position, col->position) < collectionDistance) {
			numCollected++;

			// after collecting some picnic goodness
			if (!allCollected && (numCollected >= collectibles.size())) {
				allCollected = true;
			}
		}
	}
}

// change player location to nearest respawn location
void GameManager::respawn() {
	shared_ptr<Entity> nearest = respawnPoints[0];
	for (shared_ptr<Entity> rp : respawnPoints) {
		if(glm::distance(player->position, rp->position) < glm::distance(player->position, nearest->position))
			nearest = rp;
	}
	player->position = nearest->position;
}

void GameManager::init(shared_ptr<Entity> player, map<string, shared_ptr<Entity>> worldentities) {
	player = player;

	//grab respawn points and collectibles
	map<string, shared_ptr<Entity>>::iterator i;
	for (i = worldentities.begin(); i != worldentities.end(); i++) {

		shared_ptr<Entity> entity = i->second;
		
		// check if the current entity key is a respawn
		if (i->first.find("respawn") != std::string::npos) {
			respawnPoints.push_back(entity);
		}

		// add collectibles to list of collectibles
		if (entity->tag == "collectible") {
			collectibles.push_back(entity);
		}
	}
}

void GameManager::update() {
	checkDeath();
	checkCollected();
}