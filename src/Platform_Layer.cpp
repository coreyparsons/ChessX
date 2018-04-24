//sdl things
#include "SDL.h"
//maybe should switch some of these to stb libs, but look at positives first
#include "SDL_Image.h"
#include "SDL_Mixer.h"
#include "SDL_ttf.h"

#include "platform_layer.h"
#include "main_game.h"

//debug things that will eventually go in their own files
#if DEBUG 
#define SHOW_DEBUG_TEXT 0 //make this output text to the window and not the console

//make this so you can add a name and don't need a end, as it should work from the destructor of a struct (or maybe add this as a seperate thing for functions)
//NOTE: quite hacky, cannot be nested (could solve this by giving a name and then naming the variable the name and printing with the name)
#define BEGIN_TIMED_BLOCK unsigned long long localStart = SDL_GetPerformanceCounter();
#define END_TIMED_BLOCK cout << (((SDL_GetPerformanceCounter() - localStart) / (double)SDL_GetPerformanceFrequency()) * 1000) << "ms" << endl;
#endif

/*
TODO (Platform layer):

- do error handling for SDL things (and my things)
- export this platform layer as a visual studio template so i can use it
  - also get rid of all of the useless templates that have been made redundant
  - make sure that the maths.h is as complete as i can make it (i added more functionality when making kid and messing with opengl)

- have another look at the input handling code and see if i could get sub frame input handling
  - for example if i clicked the mouse down and up within a frame, i should be able to record that and process it
  - right now i would have to keep the mouse held for the duration of the frame for it to register a mouse click

MAYBE NOT PLATFORM LAYER

- have a look at making a debug system. this may not be inside the platform layer, but could count as a utility, the same as maths.h
  - i could also look at making this a seperate thing (maybe just try to polish it and put it in useful things)
  - i should do this with the maths.h as well

- make a string handling utility
  - i want it to handle everything with char*
  - it should be faster and better than c++'s string
  - look at other string handling utilities

- after i do everything i should comb through this and make sure it's perfect

*/

//maybe should have a keyword like inline, although it doesn't seem to affect anything so i'm okay with it here
SDL_Window* window;
SDL_Renderer* renderer;

void handleEvents(SDL_Event &event, GameInput &gameInput, bool &gameRunning);
void updateAndRender(GameInput &gameInput, float fps, GameState &gameState);
void limitFps(float fps, unsigned long long &timerStart, bool vsync);
void toggleFullscreen();

int main(int argc, char* argv[])
{
	//window info
	float fps = 60;
	bool vsync = true;
	GameInput gameInput = {};

	//init screen
	SDL_Init(SDL_INIT_EVERYTHING);
	unsigned long long timerStart = SDL_GetPerformanceCounter(); //don't know if this should be with window info
	window = SDL_CreateWindow("Moving and Stuff", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 540, SDL_WINDOW_RESIZABLE);
	if (vsync)
	{
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
		SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
		SDL_GetCurrentDisplayMode(0, &mode);
		fps = (float)mode.refresh_rate;
	}
	else
	{
		renderer = SDL_CreateRenderer(window, -1, 0);
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); //anti aliasing of sorts
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); //for rendering transparent rects
	SDL_RenderSetLogicalSize(renderer, 1920, 1080);
	SDL_SetWindowIcon(window, IMG_Load("resources/icon.png")); //should this be ico? (to save having an ico (needed anyway) and a png)
	TTF_Init();

	SDL_Event event;
	GameState gameState = {}; //should this be allocated on the heap?

	while (gameState.gameRunning)
	{
		handleEvents(event, gameInput, gameState.gameRunning);
		
		updateAndRender(gameInput, fps, gameState);
		SDL_RenderPresent(renderer); //don't know if i like this here

		limitFps(fps, timerStart, vsync);
	}

	SDL_Quit();
	return 0;
}

//TODO
//this should not be in the platform layer as it contains info about the game (the gameState)
//it should be in a meta.cpp with the definition in meta.h, which can be included here to make the funciton call
inline void updateAndRender(GameInput &gameInput, float fps, GameState &gameState)
{
	switch (gameState.screen)
	{
	case GS_MAIN_GAME:
	{
		mainUpdateAndRender(gameInput, fps, gameState);
		break;
	}
	default:
	{
		//error handling here
		drawRect(0, 0, 1920, 1080, 255, 0, 0);
		SDL_RenderPresent(renderer);
		break;
	}
	}
}

inline void handleEvents(SDL_Event &event, GameInput &gameInput, bool &gameRunning)
{
	bool prevLeft = gameInput.mouseButton.left;
	bool prevAction = gameInput.action;
	bool prevBack = gameInput.back;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			gameRunning = false;
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
			{
				gameInput.back = true;
#if DEBUG
				gameRunning = false; //quick exit (temp, shouldn't be done in platform layer)
#endif
				break;
			}
			case SDLK_w:
			{
				gameInput.up = true;
				break;
			}
			case SDLK_s:
			{
				gameInput.down = true;
				break;
			}
			case SDLK_a:
			{
				gameInput.left = true;
				break;
			}
			case SDLK_d:
			{
				gameInput.right = true;
				break;
			}
			case SDLK_SPACE:
			{
				gameInput.action = true;
				break;
			}
			case SDLK_F11: //this shouldn't really be done in a platform layer (should be done in the game and make calls to the platform layer)
			{
				toggleFullscreen(); //therefore toggleFullscreen should be allowed to be called from the game
				break;
			}
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_w:
			{
				gameInput.up = false;
				break;
			}
			case SDLK_s:
			{
				gameInput.down = false;
				break;
			}
			case SDLK_a:
			{
				gameInput.left = false;
				break;
			}
			case SDLK_d:
			{
				gameInput.right = false;
				break;
			}
			case SDLK_SPACE:
			{
				gameInput.action = false;
				break;
			}
			case SDLK_ESCAPE:
			{
				gameInput.back = false;
				break;
			}
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
			{
				gameInput.mouseButton.left = true;
				break;
			}
			case SDL_BUTTON_MIDDLE:
			{
				gameInput.mouseButton.middle = true;
				break;
			}
			case SDL_BUTTON_RIGHT:
			{
				gameInput.mouseButton.right = true;
				break;
			}
			}
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
			{
				gameInput.mouseButton.left = false;
				break;
			}
			case SDL_BUTTON_MIDDLE:
			{
				gameInput.mouseButton.middle = false;
				break;
			}
			case SDL_BUTTON_RIGHT:
			{
				gameInput.mouseButton.right = false;
				break;
			}
			}
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			gameInput.mousePos = { (float)event.motion.x, (float)event.motion.y };
		}
	}
	//maybe there is a better way of doing this
	((!prevLeft) && gameInput.mouseButton.left) ? gameInput.mouseButton.leftClicked = true : gameInput.mouseButton.leftClicked = false;
	((!prevAction) && gameInput.action) ? gameInput.actionPressed = true : gameInput.actionPressed = false;
	((!prevBack) && gameInput.back) ? gameInput.backPressed = true : gameInput.backPressed = false;
}

inline void limitFps(float fps, unsigned long long &timerStart, bool vsync)
{

#if SHOW_DEBUG_TEXT //put this on the screen instead when i get debug.h/cpp up
	float possibleMilliseconds;
	float possibleFps;
	unsigned long long possibleCycles;
	float milliseconds;
	float fpsTimed;
	unsigned long long cycles;

	if (!vsync)
	{
		possibleMilliseconds = 1000 * ((float)(SDL_GetPerformanceCounter() - timerStart) / (float)SDL_GetPerformanceFrequency());
		possibleFps = (1.0f / ((float)(SDL_GetPerformanceCounter() - timerStart) / (float)SDL_GetPerformanceFrequency()));
		possibleCycles = (SDL_GetPerformanceCounter() - timerStart);
	}
#endif

	if (!vsync || ((SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED))
	{
		double delaySeconds = (1.0f / fps) - ((float)(SDL_GetPerformanceCounter() - timerStart) / (float)(SDL_GetPerformanceFrequency()));

		if (delaySeconds > 0)
		{
			SDL_Delay((int)(delaySeconds * 1000.0));
		}

		//maybe make this a "very accurate fps" setting (or implement this some other way, or just remove it)
		while (delaySeconds > 0)
		{
			delaySeconds = (1.0f / fps) - ((float)(SDL_GetPerformanceCounter() - timerStart) / (float)(SDL_GetPerformanceFrequency()));
		}
	}

#if SHOW_DEBUG_TEXT //maybe show some more debug info here, and then eventually when i make debug.cpp, this will work with it
	milliseconds = 1000 * ((float)(SDL_GetPerformanceCounter() - timerStart) / (float)SDL_GetPerformanceFrequency());
	fpsTimed = (1.0f / ((float)(SDL_GetPerformanceCounter() - timerStart) / (float)SDL_GetPerformanceFrequency()));
	cycles = (SDL_GetPerformanceCounter() - timerStart);

	timerStart = SDL_GetPerformanceCounter();

	//this part seems to take a whopping 7ms to do, DO NOT PUT COUT IN CODE FOR RELEASE!!!!!
	cout << "------------------------------\n";
	if (!vsync)
	{
		cout << "possible milliseconds: " << possibleMilliseconds << endl;
		cout << "possible FPS:          " << possibleFps << endl;
		cout << "possible cycles:       " << possibleCycles << endl;
		cout << endl;
	}
	cout << "milliseconds:          " << milliseconds << endl;
	cout << "FPS:                   " << fpsTimed << endl;
	cout << "cycles:                " << cycles << endl;
	cout << "------------------------------\n\n";
#endif
}

inline void toggleFullscreen()
{
	if ((SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
	{
		SDL_SetWindowFullscreen(window, 0);
	}
	else
	{
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
}

inline SDL_Rect rectFromPosAndSize(Vec2 pos, Vec2 size)
{
	SDL_Rect result = { (int)pos.x, (int)pos.y, (int)size.x, (int)size.y };
	return result;
}

//functions that can be called by the game
//----------------------------------------

/*
PLATFORM LAYER FUNCTIONALITY (so far)  - (maybe move this section to the top for visibility)

- draw rect (solid colour, outline or filled)
- loading textures
- drawing textures
- loading fonts
- rendering text
*/

void drawRect(float x, float y, float w, float h, unsigned char r, unsigned char g, unsigned char b)
{
	SDL_Rect rect = { roundToI(x), roundToI(y), roundToI(w), roundToI(h) };
	SDL_Color colour = { r, g, b, 255 };
	SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
	SDL_RenderFillRect(renderer, &rect);
}

void drawRect(Vec2 pos, Vec2 size, Colour colour)
{
	SDL_Rect rect = { roundToI(pos.x), roundToI(pos.y), roundToI(size.x), roundToI(size.y) };
	SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
	SDL_RenderFillRect(renderer, &rect);
}

void drawOutlineRect(Vec2 pos, Vec2 size, Colour colour, int thickness)
{
	SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

	SDL_Rect rect = { (int)pos.x, (int)pos.y, (int)size.x, (int)size.y };
	SDL_RenderDrawRect(renderer, &rect);

	for (int i = 1; i < thickness; i++)
	{
		rect = { rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2 };
		SDL_RenderDrawRect(renderer, &rect);
	}
}

SDL_Texture* loadTexture(char* filename)
{
	SDL_Surface* image = IMG_Load(filename);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
	SDL_FreeSurface(image);
	return texture;
}

void drawTexture(SDL_Texture* texture, Vec2 pos, Vec2 size)
{
	SDL_Rect dstRect = { roundToI(pos.x), roundToI(pos.y), roundToI(size.x), roundToI(size.y)};
	SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

void drawTexture(SDL_Texture* texture, Vec2 pos, Vec2 size, unsigned char alpha)
{
	//SDL_Rect srcRect = { 0, 0, roundToI(size.x), roundToI(size.y) };
	SDL_Rect dstRect = { roundToI(pos.x), roundToI(pos.y), roundToI(size.x), roundToI(size.y) };
	SDL_SetTextureAlphaMod(texture, alpha);
	SDL_RenderCopy(renderer, texture, NULL, &dstRect);
	SDL_SetTextureAlphaMod(texture, 255);
}

TTF_Font* loadFont(char* filename)
{
	TTF_Font* result = TTF_OpenFont(filename, 30); //second value basically translates to pixel height
	if (!result)
	{
		const char* FontLoadError = TTF_GetError();
		ASSERT(1);
	}
	return result;
}

//this is slow - could make it faster by saving all of the textures in a list along with the text that they are representing and then searching through the list before creating the texture
//or i could just do it the way everyone esle does it (i think) and add each letter to a giant texture and then render each letter individually when i need to render a word
void renderText(char* string, TTF_Font* font, Vec2 pos, Vec2 size, Colour colour)
//if one of the size values is 0, it will generate the appropriate size
{
	SDL_Color textColour = { colour.r, colour.g, colour.b, colour.a };
	SDL_Surface* img = TTF_RenderText_Blended(font, string, textColour);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
	SDL_FreeSurface(img);

	SDL_Rect rect;

	if (size.x > 0 && size.y > 0)
	{
		rect = { roundToI(pos.x), roundToI(pos.y), roundToI(size.x), roundToI(size.y) };
	}
	else if (size.x > 0 && size.y <= 0)
	{
		rect = { roundToI(pos.x), roundToI(pos.y), 0, 0 };
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
		float scale = (float)rect.w / (float)rect.h;
		rect.w = (int)size.x;
		rect.h = (int)(size.x / scale);
	}
	else if (size.y > 0 && size.x <= 0)
	{
		rect = { roundToI(pos.x), roundToI(pos.y), 0, 0 };
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
		float scale = (float)rect.h / (float)rect.w;
		rect.h = (int)size.y;
		rect.w = (int)(size.y / scale);
	}

	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}

void renderTextOrigin(char* string, TTF_Font* font, Vec2 pos, Vec2 size, Vec2 origin, Colour colour)
//render text with an origin, if the origin is 0.5,0.5 pos will be the center of the text
{
	SDL_Color textColour = { colour.r, colour.g, colour.b, colour.a };
	SDL_Surface* img = TTF_RenderText_Blended(font, string, textColour);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
	SDL_FreeSurface(img);

	SDL_Rect rect;

	if (size.x > 0 && size.y > 0)
	{
		rect = { roundToI(pos.x - size.x * origin.x), roundToI(pos.y - size.y * origin.y), roundToI(size.x), roundToI(size.y) };
	}
	else if (size.x > 0 && size.y <= 0)
	{
		rect = { roundToI(pos.x - size.x * origin.x), 0, 0, 0 };
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
		float scale = (float)rect.w / (float)rect.h;
		rect.w = roundToI(size.x);
		rect.h = roundToI(size.x / scale);
		rect.y = roundToI(pos.y - rect.h * origin.y);
	}
	else if (size.y > 0 && size.x <= 0)
	{
		rect = { 0, roundToI(pos.y - size.y * origin.y), 0, 0 };
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
		float scale = (float)rect.h / (float)rect.w;
		rect.h = roundToI(size.y);
		rect.w = roundToI(size.y / scale);
		rect.x = roundToI(pos.x - rect.w * origin.x);
	}

	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}

#if 0 //if i need to render text wrapped i should try to clean up this funtion i copied from elevate
void renderTextWrapped(SDL_Renderer* renderer, std::string string, TTF_Font* font, Vec2 pos, Vec2 sizeMod, Colour colour, int wrapLength)
{
	SDL_Color textColour = { colour.r, colour.g, colour.b, 255 };
	SDL_Surface* img = TTF_RenderText_Blended_Wrapped(font, string.c_str(), textColour, wrapLength / sizeMod.x);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
	SDL_FreeSurface(img);

	SDL_Rect rect = { roundToI(pos.x), roundToI(pos.y), 0, 0 };
	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	rect.w *= sizeMod.x;
	rect.h *= sizeMod.y;

	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}
#endif