#include "Player.h"

#include "Renderer.h"

// TODO: Used for yourself?
/*
Player::Player(Transform* transform)
{
	this->transform = transform;
	this->model = nullptr;
	cam = new Camera();
}
*/

// Used for other players
Player::Player(Transform* transform, int playerId)
{
	this->playerId = playerId;
	this->transform = transform;
	cam = new Camera();
	mustLoadModels = true;
}

Player::~Player() {
	for (Model* m : models) {
		delete m;
	}
}

void Player::draw(const glm::mat4& parent_transform, const glm::mat4& view)
{
	// don't draw null model anything if model is null
	if (mustLoadModels) return;
	if (playerId == Renderer::get().localPlayerId) return;
	model->draw(parent_transform, view);
}

void Player::update(float deltaTime)
{
	if (mustLoadModels) {
		loadModels();
		mustLoadModels = false;
	}

	model->update(deltaTime);
}

void Player::updatePlayer(PlayerState ps) {
	cam->update(ps.pos, ps.front);
	transform->setTranslate(ps.pos);

	if (mustLoadModels) return;

	if (ps.currentAnimation == AnimationID::WALK) {
		std::cout << "they see me walkin" << std::endl;
		model = models[1];
	}
	else {
		std::cout << "pew pew" << std::endl;
		model = models[0];
	}

	// TODO: transform->setRotate(ps.front);
}


// loads the model files and sets up shaders
void Player::loadModels() {

	models.push_back(new Model("res/models/finalToxicDab.dae"));
	models.push_back(new Model("res/models/finalWalk.dae"));

	model = models[0];
}