#pragma once
#include<Blit3D.h>

class Shot
{
public:
	
	glm::vec2 velocity, position;
	Sprite *sprite = NULL;
	float timeToLive = 2.f; //shots live for 2 seconds
	void Draw();
	bool Update(float seconds); //return false if shot dead (timeToLive <= 0)
	
};


class Ship
{
public:
	Sprite* shieldSprite;
	Sprite *shotSprite;
	std::vector<Sprite *> spriteList;
	glm::vec2 velocity, position;
	float angle = 0;
	float shotTimer = 0;
	float radius = 27.f;
	float radius2 = radius * radius;
	int lives = 3;
	float score = 0;
	int frameNumber = 0;
	float thrustTimer = 0;
	bool thrusting = false;
	bool turningLeft = false;
	bool turningRight = false;
	float shieldTimer = 0; // if >0, the shield up

	void Draw();
	void Update(float seconds);
	bool Shoot(std::vector<Shot> &shotList);
};

enum class AsteroidSize { SMALL, MEDIUM, LARGE };

class Asteroid
{
public:
	glm::vec2 velocity, position;
	Sprite* sprite = NULL;
	float radius2; // radius square for collision checking purposes.
	float spin; //some random amount of tumble, in degrees per second.
	float angle = 0; // to rotate sprite.
	AsteroidSize size;

	void Draw();
	void Update(float seconds);



};

Asteroid AsteroidFactory(AsteroidSize type);
void InitializeRNG();

AsteroidSize MakeRandomAsteroridSize();

//calculate distance squared between two porints
float DistanceSquared(glm::vec2 pos1, glm::vec2 pos2);

bool Collide(Shot& s, Asteroid& a);
bool Collide(Ship* s, Asteroid& a);

extern std::vector<Sprite*> explosionSpriteList;

class Explosion
{
public:
	int frameNum = 0; //which frame to display
	float frameSpeed = 1.f / 10.f; //how long a frame is displayed for
	float frameTimer = 0.f;
	glm::vec2 position;
	float scale = 3.f;

	void Draw();
	bool Update(float seconds);//return false if explosion should be removed

	Explosion(glm::vec2 location, float size);
};