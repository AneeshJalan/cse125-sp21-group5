#pragma once
#include <string>
#include <boost/serialization/string.hpp>

class Gun
{
public:
	std::string name;

	unsigned int fire_rate;
	unsigned int clip_size;
	unsigned int reload_time;
	unsigned int fire_rate_reset;
	unsigned int clip_size_reset;
	unsigned int reload_time_reset;
	unsigned int bullets_per_shot;
	float damage_per_bullet;
	float x_recoil;
	float y_recoil;

	void decrement_reload_time()
	{
		if (reload_time > 0)
		{
			reload_time--;
			if (reload_time == 0)
				clip_size = clip_size_reset;
		}
	}

	void decrement_fire_rate()
	{
		if (fire_rate > 0)
			fire_rate--;
		return;
	}

	void reload()
	{
		if (reload_time == 0)
			reload_time = reload_time_reset;
	}

	bool fire()
	{
		// Check if player needs to reload
		if (clip_size == 0)
		{
			// Don't reset during reload
			reload();
			return false;
		}

		// Don't fire until ready
		if (fire_rate > 0)
			return false;

		// Shoot bullet(s)
		clip_size -= bullets_per_shot;

		// Reset fire_rate
		fire_rate = fire_rate_reset;

		return true;
	}

	Gun()
	{
		name = "Gun";
		fire_rate = 0;
		fire_rate_reset = 0;
		clip_size = 0;
		clip_size_reset = 0;
		reload_time = 0;
		reload_time_reset = 0;
		bullets_per_shot = 0;
		damage_per_bullet = 0.0f;
		x_recoil = 0.0f;
		y_recoil = 0.0f;
	};

	template <typename Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& name;
		ar& clip_size;
		ar& reload_time;
	}
};

class Pistol : public Gun
{
public:
	Pistol()
	{
		name = "Pistol";
		fire_rate = 0;
		fire_rate_reset = 10;
		clip_size = 6;
		clip_size_reset = 6;
		reload_time = 10;
		reload_time_reset = 10;
		bullets_per_shot = 1;
		damage_per_bullet = 10.0f;
		x_recoil = 0.0f;
		y_recoil = 0.0f;
	}
};

class Shotgun : public Gun
{
public:
	Shotgun()
	{
		name = "Shotgun";
		fire_rate = 0;
		fire_rate_reset = 10;
		clip_size = 6;
		clip_size_reset = 6;
		reload_time = 10;
		reload_time_reset = 10;
		bullets_per_shot = 1;
		damage_per_bullet = 10.0f;
		x_recoil = 0.0f;
		y_recoil = 0.0f;
	}
};

class Rifle : public Gun
{
public:
	Rifle()
	{
		name = "Rifle";
		fire_rate = 0;
		fire_rate_reset = 10;
		clip_size = 6;
		clip_size_reset = 6;
		reload_time = 10;
		reload_time_reset = 10;
		bullets_per_shot = 1;
		damage_per_bullet = 10.0f;
		x_recoil = 0.0f;
		y_recoil = 0.0f;
	}
};