#ifndef PLATFORM_LAYER_H
#define PLATFORM_LAYER_H

#include "maths.h"
#include "main_game.h"

#include "sdl_ttf.h" //needs to include becuase i am passing a pointer to the funciton

struct SDL_Texture;
typedef SDL_Texture* Texture;
#include "assets.h"

#define DEBUG 1

//add assertions, this does not belong in the platform layer, it should really go in a debug.h
#if DEBUG
#define ASSERT(x) if (!(x)) {*(int*)0 = 0;}
#include <iostream>
using namespace std;
#else
#define ASSERT(x)
#endif

#define WINDOW_WIDTH 1920.0f
#define WINDOW_HEIGHT 1080.0f

//this code does not belong in a platform layer, the way casey handles this is that he allocates memory as a void*
//and then passes this void* into the gameUpdateAndRender, where it will be casted to a GameState*
#if 1

enum GameScreen
{
	GS_MAIN_GAME,
};

struct GameState //should this be allocated on the heap?
{
	MainState mainState = {};
	Assets assets = {};
	bool gameRunning = true;
	GameScreen screen = GS_MAIN_GAME;
};

#endif

struct MouseButton
{
	bool left;
	bool middle;
	bool right;
	bool leftClicked;
};

//game specific (may need to remove some things for this game)
struct GameInput
{
	bool up, down, left, right, action, actionPressed, back, backPressed;
	MouseButton mouseButton = {};
	Vec2 mousePos = {};
};

struct SDL_Rect;

void drawRect(float x, float y, float w, float h, unsigned char r, unsigned char g, unsigned char b);
void drawRect(Vec2 pos, Vec2 size, Colour colour);
void drawOutlineRect(Vec2 pos, Vec2 size, Colour colour, int thickness);

SDL_Texture* loadTexture(char* filename);
void drawTexture(SDL_Texture* texture, Vec2 pos, Vec2 size);
void drawTexture(SDL_Texture* texture, Vec2 pos, Vec2 size, unsigned char alpha);

TTF_Font* loadFont(char* filename);
void renderText(char* string, TTF_Font* font, Vec2 pos, Vec2 size, Colour colour);
void renderTextOrigin(char* string, TTF_Font* font, Vec2 pos, Vec2 size, Vec2 origin, Colour colour);
#endif