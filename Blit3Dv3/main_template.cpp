/*
	Template example for Wwise audio event setup/loading/playback.
*/
#include "Blit3D.h"

Blit3D *blit3D = NULL;

//memory leak detection
#define CRTDBG_MAP_ALLOC
//Can't do the following if using Wwise in debug mode
/*
#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif
#endif  // _DEBUG
*/
#include <stdlib.h>
#include <crtdbg.h>

#include "AudioEngine.h"

//GLOBAL DATA
Sprite *backgroundSprite = NULL; //a pointer to a background sprite
Sprite *heartSprite = NULL;		//a pointer to a heart-shaped sprite
float angle = 0; //for rotating the hearts

AudioEngine * audioE = NULL;
AkGameObjectID mainGameID = 1;
AkPlayingID roarID, drumsID;
bool drumsPaused = false;

void Init()
{
	//load our background image: the arguments are upper-left corner x, y, width to copy, height to copy, and file name.
	backgroundSprite = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\Logo.png");

	//load a sprite off of a spritesheet
	heartSprite = blit3D->MakeSprite(441, 98, 19, 16, "Media\\spritesheet.png");

	//create audio engine
	audioE = new AudioEngine;
	audioE->Init();
	audioE->SetBasePath("Media\\");

	//load banks
	audioE->LoadBank("Init.bnk");
	audioE->LoadBank("MainBank.bnk");

	//register our game objects
	audioE->RegisterGameObject(mainGameID);

	//start playing the looping drums
	//We can play events by name:
	drumsID = audioE->PlayEvent("Drums", mainGameID);
}

void DeInit(void)
{
	if (audioE != NULL) delete audioE;
}

void Update(double seconds)
{
	//must always update audio in our game loop
	audioE->ProcessAudio();

	//change the angle variable based on time passed since last update
	angle += static_cast<float>(seconds) * 45.f;
	if (angle > 360.f) angle -= 360.f;
}

void Draw(void)
{
	glClearColor(1.0f, 0.0f, 1.0f, 0.0f);	//clear colour: r,g,b,a 	
	// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw stuff here

	//draw the background in the middle of the screen
	//the arguments to Blit(0 are the x, y pixel coords to draw the center of the sprite at, 
	//starting as 0,0 in the bottom-left corner.
	backgroundSprite->Blit(1920.f / 2, 1080.f / 2);

	//rotate the heart:
	//sprites have a public var called angle that determines the rotation in degrees.
	heartSprite->angle = angle;
	//draw a bunch of hearts, with scaling
	for (int i = 0; i < 1920/90; ++i)
		heartSprite->Blit(i * 90.f + 45.f,  900.f,    3.f,      3.f);
	//                    X coord          Y coord   X scale   Y scale
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		//We can play events by name:
		roarID = audioE->PlayEvent("Roar", mainGameID);
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
	{
		//We can stop events by playing special "stop" events:
		audioE->StopEvent("Roar", mainGameID, roarID);
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		drumsPaused = !drumsPaused;
		//We can pase/resume events:
		if(!drumsPaused)
			audioE->PauseEvent("Drums", mainGameID, drumsID);
		else
			audioE->ResumeEvent("Drums", mainGameID, drumsID);
	}
}

int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	blit3D = new Blit3D(Blit3DWindowModel::DECORATEDWINDOW, 1920, 1080);

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