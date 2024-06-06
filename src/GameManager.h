#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <vector>
#include <map>
#include "glsl.h"
#include "Entity.h"

class GameManager {
	public:
		GameManager();
		void checkDeath();
		void checkCollected();
		void respawn();
		void init(shared_ptr<Entity> player, map<string, shared_ptr<Entity>> worldentities);
		void update();

	private:
		float deathHeight = -74.5;
		float collectionDistance = .01;
		shared_ptr<Entity> player;
		vector<shared_ptr<Entity>> respawnPoints;
		vector<shared_ptr<Entity>> collectibles;
		int numCollected = 0;
		bool allCollected = false;
		// add health here too? (professor wants loseable game)

};

#endif