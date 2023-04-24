#include "Ship.h"
#include <random>

extern std::mt19937 rng;

void Shot::Draw()
{
	sprite->Blit(position.x, position.y);

	
}

bool Shot::Update(float seconds)
{
	//TODO:
	// determine if we are past the end of our life time,
	//if so return false
	if (timeToLive <= 0)
		return false;
	timeToLive -= seconds;

	
	//move the shot
	position += velocity * seconds;

	//TODO:
	//wrap around
	//bounds check position
	if (position.x < 0) position.x += 1920.f;
	if (position.x > 1920) position.x -= 1920.f;
	if (position.y < 0) position.y += 1080.f;
	if (position.y > 1080) position.y -= 1080.f;
	
	return true;
}

void Ship::Draw()
{
	//change ship angle because my graphics face "up", not "right"
	spriteList[frameNumber]->angle = angle - 90;
	
	//draw ship sprite at 9 different locations for wrap-around effect
	for (float xoffset = -1920; xoffset <= 1920; xoffset += 1920)
	{
		for (float yoffset = -1080; yoffset <= 1080; yoffset += 1080)
		{
			spriteList[frameNumber]->Blit(position.x + xoffset, position.y + yoffset);
			//draw the shield if its activated
			if (shieldTimer > 0) shieldSprite->Blit(position.x + xoffset, position.y + yoffset, 1.5f, 1.5f, 0.60f);
		}
	}
		
}

void Ship::Update(float seconds)
{
	//handle turning
	if (turningLeft)
	{
		angle += 180.f * seconds;
	}

	if (turningRight)
	{
		angle -= 180.f * seconds;
	}

	if (thrusting)
	{
		//calculate facing vector
		float radians = angle * (M_PI / 180);
		glm::vec2 facing;
		facing.x = std::cos(radians);
		facing.y = std::sin(radians);

		facing *= seconds * 400.f;

		velocity += facing;

		//check if over max speed
		if (velocity.length() > 600.f)
		{
			velocity = glm::normalize(velocity) * 600.f;
		}

		thrustTimer += seconds;
		
		//animation timing
		if (thrustTimer >= 1.f / 20.f)
		{
			//change frames
			frameNumber++;
			if (frameNumber > 3)
				frameNumber = 1;

			thrustTimer -= 1.f / 20.f;
		}
	}
	else frameNumber = 0;

	//update position
	position += velocity * seconds;

	//bounds check position
	if (position.x < 0) position.x += 1920.f;
	if (position.x > 1920) position.x -= 1920.f;
	if (position.y < 0) position.y += 1080.f;
	if (position.y > 1080) position.y -= 1080.f;

	//reduce velocity due to "space friction"
	float scale = 1.f - seconds * 0.5f;
	if (scale < 0) scale = 0;
	velocity *= scale;

	//velocity could potentialy get very small:we should
	//cap it to zero when it gets really close,
	//but that is a more advanced topic that
	//requires techniques like std::isnan()
	float length = glm::length(velocity);
	if(std::isnan(length) || length < 0.00001f)
		velocity = glm::vec2(0, 0);

	
	//handle shot timer
	if (shotTimer > 0) 	shotTimer -= seconds;

	//handle shield timer
	if (shieldTimer > 0) 	shieldTimer -= seconds;
	
}

bool Ship::Shoot(std::vector<Shot> &shotList)
{
	if(shotTimer > 0) return false;
	if (shieldTimer > 0) return false;


	//TODO:
	//time to shoot!
	//reset shot timer
	shotTimer = 0.2f;
	
	//make a new shot
	Shot shot;
	

	//set the shot's sprite and position using the ship's variables
	shot.sprite = shotSprite;
	shot.position = position;
	
	//build a vector from the ship angle
	float radians = angle * (M_PI / 180);
	glm::vec2 facing;
	facing.x = std::cos(radians);
	facing.y = std::sin(radians);

	//scale up the shot velocity
	facing *= 600.f;
	shot.velocity = facing;

	//add the ship velocity 
	shot.velocity += velocity;
	
	//add the shot to the shotList
	shotList.push_back(shot);

	return true;
}

void Asteroid::Draw()
{
	sprite->angle = angle;

	for (float xoffset = -1920; xoffset <= 1920; xoffset += 1920)
	{
		for (float yoffset = -1080; yoffset <= 1080; yoffset += 1080)
		{
			sprite->Blit(position.x + xoffset, position.y + yoffset);
		}
	}

}

void Asteroid::Update(float seconds)
{
	position += velocity * seconds;

	//wrap around
	//bounds check position
	if (position.x < 0) position.x += 1920.f;
	if (position.x > 1920) position.x -= 1920.f;
	if (position.y < 0) position.y += 1080.f;
	if (position.y > 1080) position.y -= 1080.f;

	angle += spin * seconds;


}

extern Sprite* largeAsteroid;
extern Sprite* mediumAsteroid;
extern Sprite* smallAsteroid;

std::uniform_real_distribution<float> posDistX(0.f, 1920.f);
std::uniform_real_distribution<float> posDistY(0.f, 1080.f);
std::uniform_real_distribution<float> spinDist(-45.f, 45.f);
std::uniform_real_distribution<float> directionDist(0.f, 360.f);
std::uniform_real_distribution<float> largeSpeedDist(20.f, 150.f);
std::uniform_real_distribution<float> mediumSpeedDist(200.f, 300.f);
std::uniform_real_distribution<float> smallSpeedDist(350.f, 450.f);


Asteroid AsteroidFactory(AsteroidSize type)
{

	Asteroid A;

	A.angle = directionDist(rng);

	float dirAngle = directionDist(rng); //0-360 degrees direction
	A.velocity.x = cos(glm::radians(dirAngle));
	A.velocity.y = sin(glm::radians(dirAngle));


	switch (type)
	{
	case AsteroidSize::LARGE:
		A.sprite = largeAsteroid;
		A.velocity *= largeSpeedDist(rng);
		A.radius2 = (200.f / 2) * (200.f / 2);
		break;
	case AsteroidSize::MEDIUM:
		A.sprite = mediumAsteroid;
		A.velocity *= mediumSpeedDist(rng);
		A.radius2 = (100.f / 2) * (100.f / 2);
		break;
	case AsteroidSize::SMALL:
		A.sprite = smallAsteroid;
		A.velocity *= smallSpeedDist(rng);
		A.radius2 = (50.f / 2) * (50.f / 2);
		break;


	}

	A.size = type;

	A.position.x = posDistX(rng);
	A.position.y = posDistY(rng);

	A.spin = spinDist(rng);

	return A;
}

void InitializeRNG()
{
	std::random_device rd;
	rng.seed(rd());
}

std::uniform_int_distribution <int> asteroidSizeDist(0, 2);

AsteroidSize MakeRandomAsteroridSize()
{
	int size = asteroidSizeDist(rng);

	switch (size)

	{
	case 0:
		return AsteroidSize::SMALL;
		break;
	case 1:
		return AsteroidSize::MEDIUM;
		break;
	case 2:
		return AsteroidSize::LARGE;
		break;
	}

}

float DistanceSquared(glm::vec2 pos1, glm::vec2 pos2)
{
	return (pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y);
}


// returns true if ther was a collision
bool Collide(Shot& s, Asteroid& a)
{
	//point to circle collision check
	if (DistanceSquared(s.position, a.position) > a.radius2) return false;
	return true;
}


bool Collide(Ship* s, Asteroid& a)
{
	if (s->shieldTimer > 0) return false;
	if (DistanceSquared(s->position, a.position) > a.radius2 + s->radius2) return false;
		return true;
}

void Explosion::Draw()
{
	explosionSpriteList[frameNum]->Blit(position.x, position.y, scale, scale);
}

bool Explosion::Update(float seconds)
{
	//update frame timer
	frameTimer += seconds;
	//see if we should advance the frame counter
	if (frameTimer >= frameSpeed)
	{
		if (frameNum > explosionSpriteList.size() - 2) return false;
		frameNum++; //advance to the next frame

		frameTimer -= frameSpeed;
	}

	return true;
}

Explosion::Explosion(glm::vec2 location, float size)
{
	position = location;
	scale = size;
}