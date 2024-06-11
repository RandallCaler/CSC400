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

		int numCollected = 0;
		vector<shared_ptr<Entity>> collectibles;


	private:
		float deathHeight = -36.75f;
		float collectionDistance = .01;
		shared_ptr<Entity> player;
		vector<shared_ptr<Entity>> respawnPoints;
		bool allCollected = false;
		// add health here too? (professor wants loseable game)

};

#endif