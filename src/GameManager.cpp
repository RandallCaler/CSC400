#include "GameManager.h"
#include <algorithm>

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

void GameManager::updateRespawnPoint() {
	for (shared_ptr<Entity> rp : respawnPoints) {
		if (glm::distance(player->position, rp->position) <= CHKPT_THRESHOLD) {
			cur_rp = rp;
			return;
		}
	}
}

// change player location to nearest respawn location
void GameManager::respawn() {
	player->position = cur_rp->position;
}

void GameManager::init(shared_ptr<Entity> playerIn, map<string, shared_ptr<Entity>> worldentities) {
	player = playerIn;
	if (player != NULL) {
		//cout << "manchot has been found in game manager." << endl;
	}

	//grab respawn points and collectibles
	map<string, shared_ptr<Entity>>::iterator i;
	for (i = worldentities.begin(); i != worldentities.end(); i++) {

		shared_ptr<Entity> entity = i->second;
		
		// check if the current entity key is a respawn
		if (entity->tag == "respawn") {
			respawnPoints.push_back(entity);
			//cout << "respawn added to game manager respawn list." << endl;
			if (i->first == "respawn_init") {
				cur_rp = i->second;
			}
		}

		// add collectibles to list of collectibles
		else if (entity->tag == "collectible") {
			collectibles.push_back(entity);
			//cout << "collectible added to game manager collectible list." << endl;
		}
	}
}

void GameManager::update() {
	checkDeath();
	checkCollected();
	updateRespawnPoint();
}