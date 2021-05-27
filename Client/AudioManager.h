#pragma once

#include <fmod.hpp>
#include <unordered_map>

#define SOUND_WOOF "./res/sounds/woof.ogg"
#define SOUND_MEOW "./res/sounds/meow.ogg"
#define SOUND_STEP "./res/sounds/step.ogg"
#define SOUND_SHOOT "./res/sounds/shoot.ogg"
#define SOUND_DEATH "./res/sounds/death.ogg"
class AudioManager
{
public:

	// START SINGLETON STUFF
	static AudioManager& get() {
		static AudioManager instance;
		return instance;
	}

	// these are all needed to make sure we don't get copies of the renderer singleton
	AudioManager(AudioManager const&) = delete;
	void operator=(AudioManager const&) = delete;
	// END SINGLETON STUFF

	void init();
	void playSound(std::string soundName);

	FMOD::System* system;

private:
	AudioManager() {}

	std::unordered_map<std::string, FMOD::Sound*> mSounds;

	void loadSound(std::string filePath);
};
