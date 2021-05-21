#pragma once
#include <vector>
#include "ServerPlayer.h"
#include "../Shared/Event.h"
#include "../Shared/GameState.h"
#include "../Shared/MapState.h"
#include "Quadtree.h"

using namespace std;

class ServerGameManager
{
public:
	// To keep track of all the particles/hitboxes 
	// TODO: need to be able to make this generic 
	vector<Collider*> allColliders;

	// Quadtree variable 
	Quadtree* qt;
	Collider* tileC;

	vector<ServerPlayer> players;
	ServerGameManager();
	void handleEvent(Event& e, int playerId);
	void buildQuadtree();
	GameState getGameState(int playerId);
	MapState generateMap();

	int tileSeed;
};

