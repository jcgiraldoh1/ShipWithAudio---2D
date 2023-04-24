/*
	Asteroids ship movement example
*/

//memory leak detection
//#define CRTDBG_MAP_ALLOC
//#ifdef _DEBUG
//	#ifndef DBG_NEW
//		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//		#define new DBG_NEW
//	#endif
//#endif  // _DEBUG

#include <stdlib.h>
#include <crtdbg.h>

#include "Blit3D.h"
#include "HiScore.h"
#include "Ship.h"
#include <random>

#include "AudioEngine.h"

Blit3D *blit3D = NULL;

//GLOBAL DATA
std::mt19937 rng;
double elapsedTime = 0;
float timeSlice = 1.f / 120.f;

Sprite *backgroundSprite = NULL; //a pointer to a background sprite
Sprite* titleScreenSprite = NULL;
Ship *ship = NULL;

bool shoot = false; //should we try to shoot?
std::vector<Shot> shotList;

std::vector<Asteroid> asteroidList;

Sprite* largeAsteroid = NULL;
Sprite* mediumAsteroid = NULL;
Sprite* smallAsteroid = NULL;
AngelcodeFont* blaster3dfont = NULL;
AngelcodeFont* zetasentry3dfont = NULL;
AngelcodeFont* scorefont = NULL;


extern std::uniform_real_distribution <float> spinDist2(-45.f, 45.f);
extern std::uniform_real_distribution <float> mediumSpeedDist2(200.f, 300.f);
extern std::uniform_real_distribution <float> smallSpeedDist2(350.f, 450.f);
extern std::uniform_real_distribution <float> angleDist2(glm::radians(5.f), glm::radians(30.f));
extern std::uniform_real_distribution <float> angleDist3(glm::radians(35.f), glm::radians(60.f));
extern std::uniform_real_distribution <float> directionDist2(0.f, 360.f);

std::vector<Sprite*> explosionSpriteList;
std::vector<Explosion> explosionList;
int level = 0;
int score = 0;
enum class GameMode {STARTGAME, PLAYING, PAUSE, GAMEOVER};
GameMode gameMode = GameMode::STARTGAME;

Sprite* lifeSprite = NULL;

ScoreTable scoreTable;




void MakeLevel()
{

	level++;
	

	//turn on invulnerability - active the players shield
	ship->shieldTimer = 3.f;

	//move the ship back to the center of the screen
	ship->position = glm::vec2(1920.f / 2, 1080.f / 2);
	ship->angle = 90;
	ship->velocity = glm::vec2(0.f, 0.f);

	//cleanup old shots, asteroids
	shotList.clear();
	asteroidList.clear();
	explosionList.clear();

	for (int i = 0; i < level +3; ++i)
	{
		asteroidList.push_back(AsteroidFactory(MakeRandomAsteroridSize()));
		//asteroidList.push_back(AsteroidFactory(AsteroidSize::LARGE));
	}

}


AudioEngine* audioE = NULL;
AkGameObjectID mainGameID = 1;
AkPlayingID shootID, explosionID, thrustID, musicID;
bool engineResumed = false;


void Init()
{
	//seed rng
	std::random_device rd;
	rng.seed(rd());


	//load our hiscore table
	scoreTable.Load();

	//turn cursor off
	blit3D->ShowCursor(false);

	//load counter lifes
	lifeSprite = blit3D->MakeSprite(0, 0, 35, 58, "Media\\lives.png");

	//load our background image: the arguments are upper-left corner x, y, width to copy, height to copy, and file name.
	backgroundSprite = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\back.png");

	//load tittle screen
	titleScreenSprite = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\titleScreen.png");

	//create a ship
	ship = new Ship;
	//load a sprite off of a spritesheet
	for (int i = 0; i < 4; ++i)
		ship->spriteList.push_back(blit3D->MakeSprite(i * 72, 0, 72, 88, "Media\\Player_Ship2c.png"));

	ship->position = glm::vec2(1920.f / 2, 1080.f / 2);

	//load the shot graphic
	ship->shotSprite = blit3D->MakeSprite(0, 0, 8, 8, "Media\\shot.png");

	//load the shield graphic
	ship->shieldSprite = blit3D->MakeSprite(0, 0, 60, 60, "Media\\shield.png");
	
	//load an Angelcode binary32 font file
	blaster3dfont = blit3D->MakeAngelcodeFontFromBinary32("Media\\4114blaster3d.bin");
	zetasentry3dfont = blit3D->MakeAngelcodeFontFromBinary32("Media\\zetasentry3d.bin");

	//score sprite
	scorefont = blit3D->MakeAngelcodeFontFromBinary32("Media\\Score.bin");


	//load our asteroid graphics
	largeAsteroid = blit3D->MakeSprite(0, 0, 200, 200, "Media\\big.png");
	mediumAsteroid = blit3D->MakeSprite(0, 0, 100, 100, "Media\\medium.png");
	smallAsteroid = blit3D->MakeSprite(0, 0, 128, 128, "Media\\mini.png");

	//set the clear colour
	glClearColor(1.0f, 0.0f, 1.0f, 0.0f);	//clear colour: r,g,b,a 	

	//create audio engine
	audioE = new AudioEngine;
	audioE->Init();
	audioE->SetBasePath("Media\\");

	//load banks
	audioE->LoadBank("Init.bnk");
	audioE->LoadBank("Main.bnk");

	//register our game objects
	audioE->RegisterGameObject(mainGameID);

	//start playing the engine sound
	//We can play events by name:
	thrustID = audioE->PlayEvent("Thrust", mainGameID);
	audioE->PauseEvent("Thrust", mainGameID, thrustID);

	//start playing streaming music
	//We can play events by name:
	musicID = audioE->PlayEvent("BMusic", mainGameID);

	

	//load explosion graphics
	for (int i = 0; i < 7; ++i)
	{
		explosionSpriteList.push_back(blit3D->MakeSprite(78 + i * 40, 9, 30, 30, "Media\\Explosion.png"));
	}
}

void DeInit(void)
{
	if(ship != NULL) delete ship;

	//for (auto& S : shotList) delete& S;

	if (audioE != NULL) delete audioE;
	//any sprites/fonts still allocated are freed automatically by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	//only update time to a maximun amount - prevents big jumps in 
	//the simulation if the computer "hiccups"
	if (seconds < 0.15)
		elapsedTime += seconds;
	else elapsedTime += 0.15;

	//must always update audio in our game loop
	audioE->ProcessAudio();

	switch (gameMode)
	{
	case GameMode::PLAYING:
		
		//handle ship thrusting noise
		if (ship->thrusting)
		{
			if (!engineResumed)
			{			//unpause the engine event
				audioE->ResumeEvent("Thrust", mainGameID, thrustID);
				engineResumed = true;
			}
		}
		else
		{
			if (engineResumed)
			{
				//pause the engine event
				audioE->PauseEvent("Thrust", mainGameID, thrustID);
				engineResumed = false;
			}
		}
		//update by a full timeslice when it's time
		while (elapsedTime >= timeSlice)
		{
			elapsedTime -= timeSlice;
			ship->Update(timeSlice);


			//update the Wwise parameter for "Shipx", 
			//which is mapped to pan it left/right
			AKRESULT result = AK::SoundEngine::SetRTPCValue(L"shipx", (AkRtpcValue)(ship->position.x), mainGameID);
			assert(result == AK_Success);
			result = AK::SoundEngine::SetRTPCValue(L"shipy", (AkRtpcValue)(ship->position.y), mainGameID);
			assert(result == AK_Success);

			if (shoot)
			{
				if (ship->Shoot(shotList))
				{
					//We can play events by name:
					shootID = audioE->PlayEvent("LaserShot", mainGameID);
				}
			}

			//iterate backwards through the shotlist,
			//so we can erase shots without messing up the vector
			//for the next loop
			for (int i = shotList.size() - 1; i >= 0; --i)
			{
				//shot Update() returns false when the bullet should be killed off
				if (!shotList[i].Update(timeSlice))	
					shotList.erase(shotList.begin() + i);
				
			}

			//same explosions, itare backwards throug the vector so we can safely remove one
			for (int i = explosionList.size() - 1; i >= 0; --i)
			{
				if (!explosionList[i].Update(timeSlice))
				{
					explosionList.erase(explosionList.begin() + i);
				}
			}

			for (auto& A : asteroidList) A.Update(timeSlice);

			//collision check beetwen shots and asteriords
			for (int aIndex = asteroidList.size() - 1; aIndex >= 0; --aIndex)
			{
				for (int sIndex = shotList.size() - 1; sIndex >= 0; --sIndex)
				{
					//check for collision
					if (Collide(shotList[sIndex], asteroidList[aIndex]))
					{
						//there was a collision
						//remove the shot
						shotList.erase(shotList.begin() + sIndex);

						//We can play events by name:
						explosionID = audioE->PlayEvent("Explosion", mainGameID);


						//handle the asteroid erase/split
						switch (asteroidList[aIndex].size)
						{
						case AsteroidSize::LARGE:
						{
							//make an explosion here
							Explosion e(asteroidList[aIndex].position, 6.f);
							explosionList.push_back(e);
							//make score
							score += 10;
							
							//make a couple of medium asteroidss
							//calculate the original asterois angle of motion
							float angle = atan2f(asteroidList[aIndex].velocity.y, asteroidList[aIndex].velocity.x);

							for (int i = 0; i < 2; ++i)
							{
								float angleThisAsteroid = angle;
								Asteroid A;
								A.size = AsteroidSize::MEDIUM;
								A.position = asteroidList[aIndex].position;

								A.spin = spinDist2(rng);
								A.radius2 = (100.f / 2) * (100.f / 2);
								A.sprite = mediumAsteroid;
								A.angle = directionDist2(rng);

								//perturb the velocity
								if (i == 0) angleThisAsteroid -= angleDist2(rng);
								else angleThisAsteroid += angleDist2(rng);




								//create a velocity from this angle
								A.velocity.x = cos(angleThisAsteroid);
								A.velocity.y = sin(angleThisAsteroid);

								A.velocity *= mediumSpeedDist2(rng);

								asteroidList.push_back(A);


							}
						}
						break;
						case AsteroidSize::MEDIUM:
							//make a couple of small asteroids
						{
							Explosion e(asteroidList[aIndex].position, 4.f);
							explosionList.push_back(e);
							//make score
							score += 20;
							//calculate the original asterois angle of motion
							float angle = atan2f(asteroidList[aIndex].velocity.y, asteroidList[aIndex].velocity.x);

							for (int i = 0; i < 2; ++i)
							{
								float angleThisAsteroid = angle;
								Asteroid A;
								A.size = AsteroidSize::SMALL;
								A.position = asteroidList[aIndex].position;

								A.spin = spinDist2(rng);
								A.radius2 = (50.f / 2) * (50.f / 2);
								A.sprite = smallAsteroid;
								A.angle = directionDist2(rng);

								//perturb the velocity
								if (i == 0) angleThisAsteroid -= angleDist2(rng);
								else angleThisAsteroid += angleDist2(rng);




								//create a velocity from this angle
								A.velocity.x = cos(angleThisAsteroid);
								A.velocity.y = sin(angleThisAsteroid);

								A.velocity *= smallSpeedDist2(rng);

								asteroidList.push_back(A);

							}
						}
						break;
						default: //small asteroids
						{
							Explosion e(asteroidList[aIndex].position, 2.f);
							explosionList.push_back(e);
							//make score
							score += 70;
						}
						break;
						}//end switch
						asteroidList.erase(asteroidList.begin() + aIndex);

						//advance to next asteroid
						break;
					}
				}
			}// end of collision loop, asteroids vsshots

			if (asteroidList.empty())
			{
				MakeLevel();
				gameMode = GameMode::PAUSE;
				break;
			}

			for (auto& A: asteroidList)
			{
				//check for collision with the ship 
				if (Collide(ship, A))
				{
					//take away a life
					ship->lives--;
					//check for game over
					if (ship->lives < 1)
					{
						gameMode = GameMode::GAMEOVER;
						break;
					}

					ship->shieldTimer = 3.f;



					//make an explision ship
					Explosion e(ship->position, 3.f);
					explosionList.push_back(e);

					//We can play events by name:
					explosionID = audioE->PlayEvent("Explosion", mainGameID);

					//move the ship back to the center of the screen
					ship->position = glm::vec2(1920.f / 2, 1080.f / 2);
					ship->angle = 90;
					ship->velocity = glm::vec2(0.f, 0.f);

				}
			}

		}
		break;

	case GameMode::STARTGAME:
		level = 0;
		score = 0;
		ship->lives = 3;
		MakeLevel();
		break;
		
		gameMode = GameMode::PAUSE;
		break;
			
	case GameMode::GAMEOVER:
	default:
		break;
	}//end switch(gameMode)
	
}

void Draw(void)
{


	// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float width;
	//text next!
	std::string someText = "Asteroids";
	std::string someText1 = "Score:";
	std::string someText2 = "Level:";



	//draw stuff here
	switch (gameMode)
	{

	case GameMode::STARTGAME:

		//draw tittlescreen
		titleScreenSprite->Blit(1920 / 2, 1080 / 2);
		break;

	case GameMode::PAUSE:
	{
		//draw the background in the middle of the screen
	//the arguments to Blit(0 are the x, y pixel coords to draw the center of the sprite at, 
	//starting as 0,0 in the bottom-left corner.
		backgroundSprite->Blit(1920.f / 2, 1080.f / 2);


		//we can get the width in pixels of a string by doing the following:
		width = blaster3dfont->WidthText(someText);

				
		//draw the asteroids
		for (auto& A : asteroidList) A.Draw();

		//draw Explosion
		for (auto& E : explosionList) E.Draw();

		//draw our counter lifes HUD
		for (int i = 0; i < ship->lives; ++i)
		{
			lifeSprite->Blit(100 + i * 32, 50);
		}

		
		//draw the ship
		ship->Draw();

		//draw the shots
		for (auto& S : shotList) S.Draw();
		

		std::string paused = "PAUSED - press any key to cotinue";
		float widthText = zetasentry3dfont->WidthText(someText);
		zetasentry3dfont->BlitText(1920.f / 2.f - width / 0.6f, 700.f / 2.f, paused);

		scoreTable.Draw();
		
	}
	break;

	case GameMode::GAMEOVER:
	{

		//draw the background in the middle of the screen
	//the arguments to Blit(0 are the x, y pixel coords to draw the center of the sprite at, 
	//starting as 0,0 in the bottom-left corner.
		backgroundSprite->Blit(1920.f / 2, 1080.f / 2);
		

		//we can get the width in pixels of a string by doing the following:
		width = blaster3dfont->WidthText(someText);


		//draw the asteroids
		for (auto& A : asteroidList) A.Draw();

		//draw Explosion
		for (auto& E : explosionList) E.Draw();

		//draw our counter lifes HUD
		for (int i = 0; i < ship->lives; ++i)
		{
			lifeSprite->Blit(100 + i * 32, 50);
		}


		//draw the ship
		ship->Draw();

		//draw the shots
		for (auto& S : shotList) S.Draw();
		

		std::string gameOver = "GAME OVER";
		float widthText = zetasentry3dfont->WidthText(gameOver);
		zetasentry3dfont->BlitText(1920.f / 2.f - widthText / 2.f, 600.f / 2.f, gameOver);
		
		std::string gameOver2 = "Press G for start a new game.";
		float widthText2 = zetasentry3dfont->WidthText(gameOver2);
		zetasentry3dfont->BlitText(1920.f / 2.f - widthText2 / 2.f, 600.f / 2.f -100.f, gameOver2);

		scoreTable.SortTable();
		scoreTable.Draw();
		
	}

	break;

		
	case GameMode::PLAYING:
	

		//draw the background in the middle of the screen
	//the arguments to Blit(0 are the x, y pixel coords to draw the center of the sprite at, 
	//starting as 0,0 in the bottom-left corner.
		backgroundSprite->Blit(1920.f / 2, 1080.f / 2);


		//we can get the width in pixels of a string by doing the following:
		width = blaster3dfont->WidthText(someText);


		//draw level
		//scorefont->BlitText(1670.f / 1 - width / 2, 1080.f / 1, someText2);

		std::string someText2 = "Level:" + std::to_string(level);
		float widthText5 = scorefont->WidthText(someText2);
		scorefont->BlitText(1600, 1080, someText2);

		std::string level = "Level:";
		float widthText6 = scorefont->WidthText(level);
		scorefont->BlitText(1600, 1080, level);


		//draw the asteroids
		for (auto& A : asteroidList) A.Draw();

		//draw Explosion
		for (auto& E : explosionList) E.Draw();

		//draw our counter lifes HUD
		for (int i = 0; i < ship->lives; ++i)
		{
			lifeSprite->Blit(100 + i * 32, 50);
		}

		
		//draw score
		std::string someText1 = "Score:" + std::to_string(score);
		float widthText3 = scorefont->WidthText(someText1);
		scorefont->BlitText(0, 1080, someText1);

		std::string score = "Score:";
		float widthText4 = scorefont->WidthText(score);
		scorefont->BlitText(0, 1080, score);

		//draw the ship
		ship->Draw();

		//draw the shots
		for (auto& S : shotList) S.Draw();
		break;
	}//end switch(gameMode)
}

	

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	switch (gameMode)
	{
	case GameMode::GAMEOVER:
		if (key == GLFW_KEY_G && action == GLFW_PRESS)
			gameMode = GameMode::STARTGAME;
		break;

	case GameMode::PAUSE:
		gameMode= GameMode::PLAYING;
		//fall through to  PLAYING state
	case GameMode::PLAYING:
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			blit3D->Quit(); //start the shutdown sequence

		if (key == GLFW_KEY_A && action == GLFW_PRESS)
			ship->turningLeft = true;

		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
			ship->turningLeft = false;

		if (key == GLFW_KEY_D && action == GLFW_PRESS)
			ship->turningRight = true;

		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
			ship->turningRight = false;

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
			ship->thrusting = true;

		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
			ship->thrusting = false;

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
			shoot = true;

		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
			shoot = false;

		if (key == GLFW_KEY_P && action == GLFW_RELEASE)
			gameMode = GameMode::PAUSE;
		break;

	case GameMode::STARTGAME:
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
			gameMode = GameMode::PLAYING;

	default:
		//gameMode = GameMode::PLAYING;
		break;
	}
	
}

int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//set X to the memory allocation number in order to force a break on the allocation:
	//useful for debugging memory leaks, as long as your memory allocations are deterministic.
	//_crtBreakAlloc = X;

	blit3D = new Blit3D(Blit3DWindowModel::BORDERLESSFULLSCREEN_1080P, 1920, 1080);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}