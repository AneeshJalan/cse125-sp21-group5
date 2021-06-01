// Control Networking Packet size
#define PACKET_SIZE 2048

// Control number of players in game.
#define NUM_PLAYERS 2

// Control Map Generation
#define TILE_SIZE 20
#define NUM_MAP_TILES 5
#define MAX_NUM_TREES_PER_TILE 5

// Control Player Properties
#define DEATH_TICK_TIMER 100
#define DOG_FLAG_SPAWN glm::vec3(TILE_SIZE * (NUM_MAP_TILES / 2), 1, TILE_SIZE * (NUM_MAP_TILES / 2))
#define CAT_FLAG_SPAWN glm::vec3(TILE_SIZE * (-NUM_MAP_TILES / 2), 1, TILE_SIZE * (-NUM_MAP_TILES / 2))
#define CAT_SPAWN glm::vec3(TILE_SIZE * (NUM_MAP_TILES / 2) + 5, 1.5f, TILE_SIZE * (NUM_MAP_TILES / 2) + 5)
#define DOG_SPAWN glm::vec3(TILE_SIZE * (-NUM_MAP_TILES / 2) - 5, 1.5f, TILE_SIZE * (-NUM_MAP_TILES / 2) - 5)

// Game properties 
#define NUM_CAPTURES_TO_WIN 1
#define VISION_IMPAIRMENT_TIME 300
#define FREEZE_TIME 180

enum class PlayerTeam
{
	CAT_LOVER = 0,
	DOG_LOVER = 1,
};