#include "ServerGameManager.h"
#include <glm/gtx/string_cast.hpp>


ServerGameManager::ServerGameManager() {

	// TODO: remove, hardcoded initPos
	// TODO: add new player colliders as players connect
	tileSeed = (int)time(NULL);

	//Tile and flags are unintialized
	flagCatCarrierId = -1;
	flagDogCarrierId = -1;
}

MapState ServerGameManager::generateMap()
{
	// Set the random seed, so thing generate the same way
	srand(tileSeed);

	// Create one massive tile for entirety of map
	Collider* tileC = new Collider(ObjectType::GROUND, glm::vec3(0.0f, -2.5f, 0.0f), glm::vec3(TILE_SIZE * NUM_MAP_TILES, 5.0f, TILE_SIZE * NUM_MAP_TILES));
	allColliders.push_back(tileC);

	for (int i = 0; i < NUM_MAP_TILES; i++)
	{
		for (int j = 0; j < NUM_MAP_TILES; j++)
		{
			// Center point of the tile
			glm::vec3 tileCenter = glm::vec3(TILE_SIZE * (i - NUM_MAP_TILES / 2), 0.0f, TILE_SIZE * (j - NUM_MAP_TILES / 2));

			// Special case for flags
			if (i == 0 && j == 0)
			{
				flagCat = new Collider(ObjectType::FLAG_CAT, tileCenter + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f));
				allColliders.push_back(flagCat);
				continue;
			}
			else if (i == NUM_MAP_TILES - 1 && j == NUM_MAP_TILES - 1)
			{
				flagDog = new Collider(ObjectType::FLAG_DOG, tileCenter + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f));
				allColliders.push_back(flagDog);
				continue;
			}

			// Generate Trees
			int numTrees = rand() % MAX_NUM_TREES_PER_TILE;
			for (int k = 0; k < numTrees; k++)
			{
				// Pick random position within tile
				float x = TILE_SIZE * (rand() / (float)RAND_MAX) - 10.0f;
				float z = TILE_SIZE * (rand() / (float)RAND_MAX) - 10.0f;

				// Make tree collider
				Collider* treeC = new Collider(ObjectType::ENVIRONMENT, glm::vec3(x, 5.0f, z) + tileCenter, glm::vec3(1.0f, 10.0f, 1.0f));
				rand(); // for random tree models on client
				rand(); // for rotations on client
				allColliders.push_back(treeC);
			}
		}
	}

	// Create capture areas at opposite corners
	dogWinArea = new Collider(ObjectType::ENVIRONMENT, flagCat->cen, flagCat->dim + glm::vec3(1.0f));
	catWinArea = new Collider(ObjectType::ENVIRONMENT, flagDog->cen, flagDog->dim + glm::vec3(1.0f));

	// Set up quadtree 
	Collider boundary = Collider(glm::vec3(0, -5.0f, 0), glm::vec3(110.0f, 50.0f, 110.0f));
	qt = new Quadtree(boundary, 4);
	for (Collider* c : allColliders)
	{
		qt->insert(c);
	}

	// Create map state
	return MapState(tileSeed);
}

void ServerGameManager::handleShoot(ServerPlayer* player)
{
	// Select current gun
	Gun* gun = player->guns[player->gun_idx];

	// Check gun stuff
	if (!gun->fire())
	{
		return;
	}

	// Trace each bullet
	for (int i = 0; i < gun->bullets_per_shot; i++)
	{
		// For calculating min distance
		float minHitlength = std::numeric_limits<float>::infinity();
		Collider* closestCollider = nullptr;

		// Check shooting against all other colliders before checking movement 
		for (Collider* otherCollider : allColliders)
		{
			// Ignore collisions with yourself
			if (player->hitbox == otherCollider)
				continue;

			// Check for shooting stuff
			glm::vec3 hitPos;
			if (otherCollider->check_ray_collision(player->hitbox->cen, player->front, hitPos))
			{
				float hitLength = glm::length(hitPos - player->hitbox->cen);
				if (hitLength < minHitlength)
				{
					minHitlength = hitLength;
					closestCollider = otherCollider;
				}
			}
		}

		// Handle Hit Damage
		if (closestCollider != nullptr && closestCollider->type == ObjectType::PLAYER)
		{
			// TODO: maybe find constant time way to do this
			for (auto p : players)
			{
				if (p.second->hitbox == closestCollider)
				{
					p.second->decreaseHealth(gun->damage_per_bullet);

					// Check if player died
					if (p.second->isDeadCheck())
					{
						// Set death timer
						p.second->isDead = DEATH_TICK_TIMER;

						// Disable hitbox
						p.second->hitbox->isActive = false;

						// Increase death count
						p.second->deaths++;

						// Drop corresponding flag
						if (flagCatCarrierId == p.first)
						{
							flagCatCarrierId = -1;
							flagCat->isActive = true;
							flagCat->set_center(p.second->pos - 2.0f * p.second->front);
						}
						else if (flagDogCarrierId == p.first)
						{
							flagDogCarrierId = -1;
							flagDog->isActive = true;
							flagDog->set_center(p.second->pos - 2.0f * p.second->front);
						}
						buildQuadtree();

						// Increment kill count
						player->kills++;
					}
					break;
				}
			}
		}
	}
}

void ServerGameManager::handleEvent(Event& e, int playerId)
{
	// Get the current player
	ServerPlayer* curr_player = players[playerId];

	// TODO: Varying death timers 
	if (curr_player->isDead > 0)
	{
		// Update animation for death
		players[playerId]->animation = AnimationID::DEATH;
		players[playerId]->isDead--;
		
		// Reset variables when they are alive again
		if (players[playerId]->isDead == 0)
		{
			// TODO: Reset player (including gun variables)
			players[playerId]->health = 100.0f;
			players[playerId]->hitbox->isActive = true;

			// TP to cat base
			if (players[playerId]->team == PlayerTeam::CAT_LOVER) {
				players[playerId]->pos = CAT_SPAWN;
			}
			else if (players[playerId]->team == PlayerTeam::DOG_LOVER) {
				players[playerId]->pos = DOG_SPAWN;
			}
		}
		// If the player is dead, we don't let them move or do anything
		return;
	}

	// Switch weapons
	curr_player->gun_idx = e.gun_idx;

	// Pick animation
	curr_player->updateAnimations(e);

	// Check if player has fallen off map
	if (curr_player->pos.y < -10.0f)
	{
		// Slowly decrement health
		curr_player->decreaseHealth(10.0f);
	
		// If player dies, return flag back to area
		if (curr_player->isDeadCheck())
		{
			// Set death timer
			curr_player->isDead = DEATH_TICK_TIMER;

			// Disable hitbox
			curr_player->hitbox->isActive = false;

			// Increase death count
			curr_player->deaths++;

			if (flagCatCarrierId == playerId)
			{
				flagCatCarrierId = -1;
				flagCat->set_center(CAT_FLAG_SPAWN);
				flagCat->isActive = true;
			}
			else if (flagDogCarrierId == playerId)
			{
				flagDogCarrierId = -1;
				flagDog->set_center(DOG_FLAG_SPAWN);
				flagDog->isActive = true;
			}

			buildQuadtree();
		}
	}

	// Not jumping
	if (!e.jumping)
	{
		if (curr_player->vVelocity >= 0.0f)
		{
			curr_player->vVelocity -= 0.1f;
		}
		curr_player->update(e.dPos + glm::vec3(0.0f, curr_player->vVelocity, 0.0f), e.dYaw, e.dPitch);
	}
	// Jumping 
	else {
		// 10 ticks of jumping in total
		curr_player->jumping = 10;
	}

	// Parabolic jumping
	float jumpingSquared = curr_player->jumping * curr_player->jumping;
	// Handle jumping tick by tick
	if (curr_player->jumping > 0)
	{
		curr_player->update(e.dPos + glm::vec3(0.0f, jumpingSquared/100.0f, 0.0f), e.dYaw, e.dPitch);
		// 5 ticks of jumping in total
		curr_player->jumping--;
	}

	// Rebuild quadtree for collision after player movement is updated
	buildQuadtree();

	// Update gun ticks
	for (Gun* g : curr_player->guns)
	{
		g->decrement_fire_rate();
		g->decrement_reload_time();
	}

	// Handle shooting
	if (e.shooting)
	{
		handleShoot(curr_player);
	}

	// Movement for colliders 
	Collider* queryRange = new Collider(curr_player->hitbox->cen, curr_player->hitbox->dim * 10.0f);
	vector<Collider*> nearbyColliders;
	nearbyColliders = qt->query(queryRange, nearbyColliders);
	curr_player->isGrounded = false;
	for (Collider* otherCollider : nearbyColliders)
	{
		// Ignore collisions with yourself
		if (curr_player->hitbox == otherCollider)
			continue;

		// Determine which plane collision happened on
		glm::vec3 plane = curr_player->hitbox->check_collision(otherCollider);

		// For jumping
		if (plane.y > 0.0f)
		{
			curr_player->isGrounded = true;
		}

		curr_player->update(plane, 0.0f, 0.0f);

		// If it happened on no plane
		if (plane == glm::vec3(0.0f))
			continue;

		// Pick up flag
		if (otherCollider == flagCat && flagCatCarrierId == -1 && playerId % 2 == (int)PlayerTeam::CAT_LOVER) {
			flagCatCarrierId = playerId;
			flagCat->isActive = false;
		}
		else if (otherCollider == flagDog && flagDogCarrierId == -1 && playerId % 2 == (int)PlayerTeam::DOG_LOVER) {
			flagDogCarrierId = playerId;
			flagDog->isActive = false;
		}
	}

	// Move cat with player
	if (flagCatCarrierId != -1)
	{
		flagCat->set_center(players[flagCatCarrierId]->pos - 2.0f * players[flagCatCarrierId]->front);
	}

	// Move dog with player
	if (flagDogCarrierId != -1)
	{
		flagDog->set_center(players[flagDogCarrierId]->pos - 2.0f * players[flagDogCarrierId]->front);
	}

	// Detect score for Cat team
	if (flagCatCarrierId == playerId && curr_player->team == PlayerTeam::CAT_LOVER)
	{
		glm::vec3 plane = curr_player->hitbox->check_collision(catWinArea);
		if (plane != glm::vec3(0.0f))
		{
			curr_player->captures++;

			// Reset cat flag
			flagCatCarrierId = -1;
			flagCat->set_center(CAT_FLAG_SPAWN);
		}
			
	}
	else if(flagDogCarrierId == playerId && curr_player->team == PlayerTeam::DOG_LOVER)
	{
		glm::vec3 plane = curr_player->hitbox->check_collision(dogWinArea);
		if (plane != glm::vec3(0.0f))
		{
			curr_player->captures++;

			// Reset dog flag
			flagDogCarrierId = -1;
			flagDog->set_center(DOG_FLAG_SPAWN);
		}
	}

	buildQuadtree();
}

void ServerGameManager::buildQuadtree()
{
	Collider boundary = Collider(glm::vec3(0, -5.0f, 0), glm::vec3(110.0f, 30.0f, 110.0f));
	qt = new Quadtree(boundary, 4);

	for (Collider* c : allColliders)
	{
		qt->insert(c);
	}
}

GameState ServerGameManager::getGameState(int playerId)
{
	GameState gs;

	for (int i = 0; i < players.size(); i++)
	{
		PlayerState ps(i,
			           players[i]->pos,
			           players[i]->front,
			           players[i]->animation,
			           players[i]->isGrounded,
			           players[i]->health,
			           players[i]->isDead,
					   (flagCatCarrierId == i),
					   (flagDogCarrierId == i),
			           players[i]->kills,
			           players[i]->deaths,
			           players[i]->captures,
					   players[i]->gun_idx,
					   *(players[i]->guns[players[i]->gun_idx]));

		gs.addState(ps);
	}

	// Send back flag locations
	gs.catLocation = flagCat->cen;
	gs.dogLocation = flagDog->cen;

	return gs;
}

void ServerGameManager::createNewPlayer(int playerId)
{
	glm::vec3 playerSpawnPos = (playerId % 2) == 0 ? CAT_SPAWN : DOG_SPAWN;
	players[playerId] = new ServerPlayer(playerSpawnPos, playerId);


	// Add player hitboxes to all colliders
	allColliders.push_back(players[playerId]->hitbox);
}
