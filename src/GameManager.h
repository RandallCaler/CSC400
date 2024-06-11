#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#define CHKPT_THRESHOLD 5.0f

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
		void updateRespawnPoint();
		void update();

	private:
		float deathHeight = -36.75f;
		float collectionDistance = .01;
		shared_ptr<Entity> player;
		vector<shared_ptr<Entity>> respawnPoints;
		vector<shared_ptr<Entity>> collectibles;
		int numCollected = 0;
		bool allCollected = false;
		shared_ptr<Entity> cur_rp;
		// add health here too? (professor wants loseable game)

};

#endif