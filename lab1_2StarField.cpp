// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;
vector<vec3> stars(1000);

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Update();
int t;
float v = 0.001;

// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main(int argc, char* argv[])
{   
	for (int i = 0; i < stars.size(); ++i)
	{
		float x = 2 * float(rand()) / float(RAND_MAX) - 1;
		float y = 2 * float(rand()) / float(RAND_MAX) - 1;
		float z = float(rand()) / float(RAND_MAX);
		stars[i] = vec3(x, y, z);
	}
	t = SDL_GetTicks();
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
		
	}
	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}

void Draw()
{
	SDL_FillRect(screen, 0, 0);
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	for (size_t s = 0; s<stars.size(); ++s)
	{
		// Add code for projecting and drawing each star

		vec3 color = 0.2f * vec3(1, 1, 1) / (stars[s].z*stars[s].z);
		//vec3 color = vec3(1, 1, 1);
		float x, y, z, u, v, f;
		x = stars[s][0];
		y = stars[s][1];
		z = stars[s][2];
		f = SCREEN_HEIGHT / 2;
		u = f * x / z + SCREEN_WIDTH / 2;
		v = f * y / z + SCREEN_HEIGHT / 2;
		PutPixelSDL(screen, u, v, color);
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
void Update()
{
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	for (int s = 0; s<stars.size(); ++s)
	{
		float x, y, z;
		x  = stars[s][0];
		y  = stars[s][1];
		z  = stars[s][2];
		stars[s][2] -= v*dt;
		if (stars[s].z <= 0)
			stars[s].z += 1;
		if (stars[s].z > 1)
			stars[s].z -= 1;
	}
}