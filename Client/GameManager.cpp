#include "GameManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../Shared/Global_variables.h"

#include "Renderer.h"

// TODO: possibly move these as well
// Track mouse movements
bool firstMouse = true;
float GameManager::offsetX = 0.0f;
float GameManager::offsetY = 0.0f;
float GameManager::lastX = Window::width / 2;
float GameManager::lastY = Window::height / 2;
float GameManager::fov = 60.0f;

GameManager::GameManager(GLFWwindow* window)
{
	// Save pointer to window
	this->window = window;

	// Initialize transforms
	worldT = new Transform();
	worldT->setName("World Transform");
	// Preload models
	// TODO: maybe save this in a map for less variables
	playerModel = new Model("res/models/unitCube.dae");
	playerModel->setName("Player Model");
	tileModel = new Model("res/models/finalTileZY.dae");
	tileModel->setName("Tile Model");
	treeModel = new Model("res/models/willowTrunk.dae");
	treeModel->setName("Tree Model");
	catT = nullptr;
	dogT = nullptr;
	catModel = new Model("res/models/finalZombieFish.dae");
	dogModel = new Model("res/models/finalHuskyRun.dae");

	// Initialize variables
	showScoreboard = false;

	// Add a test point light
	Renderer::get().addPointLight(PointLight(glm::vec3(0, 2, -2), glm::vec3(1, 0, 0)));
	Renderer::get().addPointLight(PointLight(glm::vec3(0, 2, 2), glm::vec3(0, 1, 0)));

	Renderer::get().addDirectionalLight(DirectionalLight(glm::vec3(1, 2, 0), glm::vec3(0.9)));

	Renderer::get().fogDensity = 0.0001;

	//Renderer::get().addSpotLight(SpotLight(glm::vec3(0, 15, 0), glm::vec3(0, 0, 1), glm::vec3(1), 30));

	// Initialize time variables
	deltaTime = 0.0f;
	prevTime = (float) glfwGetTime();
	currTime = (float) glfwGetTime();

	// Uninitialized playerID
	localPlayerId = -1;
} 

GameManager::~GameManager()
{
	// Delete all models
	delete worldT; // Recursively calls destructor for all nodes... hopefully
}

Event GameManager::update()
{
	if (localPlayerId == -1) {
		// Client has not yet connected to the server.
		cout << "Local Player ID not received yet... Waiting to connect to Server..." << endl;
		// TODO: Display Loading Screen...
		return Event();
	}
	else {
		//cout << "Player pointer in unordered_map for localPlayerID " << localPlayerId << " is " << players[localPlayerId];
	}

	// Make a new imgui frame
	// do this here so game objects can make ImGUI calls in their update function
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Calculate deltaTime
	currTime = (float) glfwGetTime();
	deltaTime = currTime - prevTime;
	prevTime = currTime;

	Renderer::get().update(deltaTime);
	worldT->update(deltaTime);

	// Rendering of objects is done here. (Draw)
	render();

	// Listen for any events (keyboard input, mouse input, etc)
	glfwPollEvents();

	// Process keyboard input
	Event e = handleInput();

	// Process gravity
	//cout << glm::to_string(players[localPlayerId]->cam->pos) << endl;

	// Update camera position
	// TODO: necessary?
	offsetX = 0.0f;
	offsetY = 0.0f;
	return e;
}

// Handle Keyboard Input
Event GameManager::handleInput()
{
	// Get current mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Special case for first mouse
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	// Calculate offset from prev frame
	offsetX = (float)(xpos - lastX);
	offsetY = (float)(lastY - ypos);

	// Save previous positions
	lastX = (float)xpos;
	lastY = (float)ypos;

	// System Controls
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Get Player Camera
	Camera* camera = players[localPlayerId]->cam;

	// Jumping control 
	bool jumping = false;

	// Player Controls
	glm::vec3 dPos = glm::vec3(0.0f);
	glm::vec3 dir(camera->front.x, 0.0f, camera->front.z);
	if (glfwGetKey(window, GLFW_KEY_W))
	{
		//cout << "before" << glm::to_string(dPos) << endl;
		//dPos += camera->front;
		dPos += glm::normalize(dir);
		//cout << "after" << glm::to_string(dPos) << endl;

	}
	else if (glfwGetKey(window, GLFW_KEY_S))
	{
		//dPos -= camera->front;
		dPos -= glm::normalize(dir);
	}
	if (glfwGetKey(window, GLFW_KEY_A)) 
	{
		//dPos += -glm::normalize(glm::cross(camera->front, camera->up));
		dPos -= glm::normalize(glm::cross(dir, camera->up));
	}
	else if (glfwGetKey(window, GLFW_KEY_D))
	{
		//dPos += glm::normalize(glm::cross(camera->front, camera->up));
		dPos += glm::normalize(glm::cross(dir, camera->up));
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE))
	{
		if(players[localPlayerId]->isGrounded)
			jumping = 5;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
	{
		dPos -= camera->up;
	}

	// Show scoreboard
	if (glfwGetKey(window, GLFW_KEY_TAB))
	{
		cerr << "showing scoreboard" << endl;
		showScoreboard = true;
	}
	else
	{
		showScoreboard = false;
	}

	// Accumulate direction, and convert to offset
	if (dPos != glm::vec3(0.0f))
		dPos = glm::normalize(dPos);
	dPos *= camera->speed * deltaTime;

	// Update mouse movements
	float yaw = camera->sensitivity * offsetX;
	float pitch = camera->sensitivity * offsetY;

	// Detect mouse presses
	bool shooting = false;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
	{
		shooting = true;
	}
	// If the player is dead, yeet
	if (players[localPlayerId]->isDead == DEATH_TICK_TIMER) {
		dPos = glm::vec3(0.0f, 15.0f, 0.0f);
	}

	return Event(dPos, yaw, pitch, shooting, jumping);
}

// Use for one-time key presses
void GameManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
	{
		Renderer::get().debug = !Renderer::get().debug;
	}
}

// Detect mouse clicks
void GameManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
}

// Detect mouse position
void GameManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
}

// Detect mouse scroll
void GameManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	cerr << "Fov: " << fov << endl;
	fov -= yoffset;
	fov = glm::clamp(fov, 1.0f, 270.0f);
	Window::projection =
		glm::perspective(glm::radians(fov), (float) Window::width / (float) Window::height, 0.1f, 1000.0f);
}

// Draw objects to screen
void GameManager::render()
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// show an example window
	ImGuiWindowFlags windowFlags = 0;

	windowFlags |= ImGuiWindowFlags_NoTitleBar;
	windowFlags |= ImGuiWindowFlags_NoScrollbar;
	windowFlags |= ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoResize;
	windowFlags |= ImGuiWindowFlags_NoCollapse;
	windowFlags |= ImGuiWindowFlags_NoNav;
	windowFlags |= ImGuiWindowFlags_NoBackground;
	windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	bool showUI = true;
	ImGui::Begin("Health UI", &showUI, windowFlags);
	ImGui::SetWindowPos(ImVec2(50, Window::height - 150));
	ImGui::SetWindowSize(ImVec2(300, 100));
	ImGui::Text("Super basic health bar");
	ImGui::ProgressBar(players[localPlayerId]->health / 100.0f);
	ImGui::End();

	// super basic crosshair, maybe move this somewhere else
	ImGui::Begin("Crosshairs", &showUI, windowFlags);
	ImGui::SetWindowPos(ImVec2(Window::width / 2 - 20, Window::height / 2 - 20));
	ImGui::SetWindowSize(ImVec2(200, 200));

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();
	drawList->AddLine(ImVec2(p.x + 20, p.y), ImVec2(p.x + 20, p.y + 40), ImColor(ImVec4(0, 1, 0, 1)), 1.0);
	drawList->AddLine(ImVec2(p.x, p.y + 20), ImVec2(p.x + 40, p.y + 20), ImColor(ImVec4(0, 1, 0, 1)), 1.0);
	ImGui::End();

	// Debug UI Information
	if (Renderer::get().debug)
	{
		ImGui::Begin("Debug UI", &showUI, windowFlags);
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(1000, 500));
		Player* p = players[localPlayerId];
		ImGui::Text("Number of Players: %d", players.size());
		ImGui::Text("Player ID: %d", localPlayerId);
		ImGui::Text("Player center position: (%.2f, %.2f, %.2f)", p->cam->pos.x, p->cam->pos.y, p->cam->pos.z);
		ImGui::Text("Player isDead: %d", p->isDead);
		ImGui::Text("Player isGrounded: %d", p->isGrounded);
		ImGui::Text("Player isCarryingFlag: %d", p->isCarryingCatFlag || p->isCarryingDogFlag);
		ImGui::End();
	}

	// Scoreboard UI
	if (showScoreboard)
	{
		ImGui::Begin("Debug UI", &showUI, windowFlags);
		ImGui::SetWindowPos(ImVec2(Window::width/2 - 250, Window::height/2 - 250));
		ImGui::SetWindowSize(ImVec2(500, 500));

		for (auto& p : players)
		{
			//ImGui::Begin("test");
			ImGui::Text("Player ID: %d", p.first);
			ImGui::Text("Kills: %d", p.second->kills);
			ImGui::Text("Deaths: %d", p.second->deaths);
			ImGui::Text("Captures: %d", p.second->captures);
			//ImGui::End();
		}
		ImGui::End();
	}
	

	//Renderer::get().setCamera(players[localPlayerId]->cam);

	// Render the models
	Renderer::get().setCamera(players[localPlayerId]->cam);
	worldT->draw(glm::mat4(1), Window::projection * players[localPlayerId]->cam->view);

	// call ImGUI render to actually render the ui to opengl
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers
	glfwSwapBuffers(window);
}

// We translate then scale on the server, so we have to match on client
void GameManager::updateMap(MapState& ms)
{
	srand(ms.tileSeed);

	for (int i = 0; i < NUM_MAP_TILES; i++)
	{
		for (int j = 0; j < NUM_MAP_TILES; j++)
		{
			// Define center of tile
			glm::vec3 tileCenter = glm::vec3(TILE_SIZE * (i - NUM_MAP_TILES / 2), 0, TILE_SIZE * (j - NUM_MAP_TILES / 2));

			// Create tile transform
			Transform* tileT = new Transform(glm::vec3(1.0f), glm::vec3(0.0f), tileCenter);
			tileT->setName("Tile ["  + std::to_string(i) + "][" + std::to_string(j) + "] Transform");
			tileT->add_child(tileModel);
			worldT->add_child(tileT);

			// Cat flag
			if (i == 0 && j == 0)
			{
				catT = new Transform(glm::vec3(1.0f), glm::vec3(0.0f), tileCenter + glm::vec3(0.0f, 0.75f, 0.0f));
				cout << "cat at: " << glm::to_string(catT->translation) << endl;
				catT->setName("catT");
				catT->add_child(catModel);
				worldT->add_child(catT);
				continue;
			}
			// Dog flag
			else if (i == NUM_MAP_TILES - 1 && j == NUM_MAP_TILES - 1)
			{
				dogT = new Transform(glm::vec3(1.0f), glm::vec3(0.0f), tileCenter + glm::vec3(0.0f, 0.75f, 0.0f));
				cout << "dog at: " << glm::to_string(dogT->translation) << endl;
				dogT->setName("dogT");
				dogT->add_child(dogModel);
				worldT->add_child(dogT);
				continue;
			}

			// Generate Trees
			int numTrees = rand() % MAX_NUM_TREES_PER_TILE;
			for (int k = 0; k < numTrees; k++) {
				// Pick random tree position
				float x = TILE_SIZE * (rand() / (float)RAND_MAX) - 10.0f;
				float z = TILE_SIZE * (rand() / (float)RAND_MAX) - 10.0f;
			    
				// Generate the position inside the tile
				Transform* treeT = new Transform(glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(x, 0.0f, z));
				treeT->add_child(treeModel);
				treeT->setName("Tree [" + std::to_string(i) + "][" + std::to_string(j) + "] Transform");
				tileT->add_child(treeT);
			}
		}
	}
}

void GameManager::updateGameState(GameState& gs)
{
	for (const PlayerState& ps : gs.states)
	{
		// TODO: Decide what to do if player doesn't exist but update is received.
		// Ignore update if player doesn't exist
		if (players.find(ps.playerId) == players.end())
			continue;

		players[ps.playerId]->updatePlayer(ps);
		// TODO: stop flags from floating?
		if (ps.carryingCatFlag)
		{
			Transform* playerT = players[ps.playerId]->transform;
			glm::vec3 directionalTrans = glm::normalize(glm::vec3(players[ps.playerId]->cam->front.x, 0.1f, players[ps.playerId]->cam->front.z));
			catT->setTranslate(playerT->translation - 2.5f * directionalTrans);
		}
		else if (ps.carryingDogFlag)
		{
			Transform* playerT = players[ps.playerId]->transform;
			glm::vec3 directionalTrans = glm::normalize(glm::vec3(players[ps.playerId]->cam->front.x, 0.1f, players[ps.playerId]->cam->front.z));
			dogT->setTranslate(playerT->translation - 2.5f * directionalTrans);
		}
	}
}

// TODO: Model* should be a string or int to what kind of model should be used to render player
void GameManager::addPlayer(int playerId, Model* playerModel)
{
	// Check if player already exists
	if (players.find(playerId) != players.end())
	{
		cerr << "Player with ID " << playerId << " already exists!" << endl;
		return;
	}

	// Create new player with model
	Transform* playerT = new Transform();
	playerT->setName("Player " + std::to_string(playerId) + " Transform");
	Player* player = new Player(playerT, playerId);
	playerT->setName("Player " + std::to_string(playerId));
	
	playerT->add_child(player);
	worldT->add_child(playerT);

	//players.insert(make_pair(playerId, player));
	players[playerId] = player;
}

void GameManager::setLocalPlayerID(int playerId)
{
	std::cout << "Local player id is " << playerId << std::endl;
	this->localPlayerId = playerId;
	Renderer::get().localPlayerId = playerId;
}