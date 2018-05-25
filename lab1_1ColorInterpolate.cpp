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

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
//void Interpolate(float a, float b, vector<float>& result);
void Interpolate_vector(vec3 a, vec3 b, vector<vec3>& result);
// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main(int argc, char* argv[])
{
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	while (NoQuitMessageSDL())
	{
		Draw();
	}
	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}
/*
void Interpolate(float a, float b, vector<float>& result)
{
int data_length = result.size();
for (int i = 0; i < data_length; ++i)
{
float step = (b - a + 1) / data_length;
result[i] = step * i + a;
}
}

*/

void Interpolate_vector(vec3 a, vec3 b, vector<vec3>& result)
{
	int data_length = sizeof(result);//.size();
	int vector_length = 3;
	for (int i = 0; i < data_length;++i)
	{
		if (data_length == 1)
		{
			printf("Invalid output length, should be an integer larger than 1 \n");
		}
		else
		{
			for (int j = 0; j < vector_length; ++j)
				{
					float step = (b[j] - a[j]) / (data_length - 1);
					result[i][j] = step * i + a[j];
				}
		}
	}
}


void Draw()
{
	vec3 topLeft(1, 0, 0); // red
	vec3 topRight(0, 0, 1); // blue
	vec3 bottomLeft(0, 1, 0); // green
	vec3 bottomRight(1, 1, 0); // yellow

	vector<vec3> leftSide(SCREEN_HEIGHT);
	vector<vec3> rightSide(SCREEN_HEIGHT);
	vector<vec3> result(SCREEN_WIDTH);

	Interpolate_vector(topLeft,  bottomLeft,  leftSide);
	Interpolate_vector(topRight, bottomRight, rightSide);
	for (int y = 0; y<SCREEN_HEIGHT; ++y)
	{
		Interpolate_vector(leftSide[y], rightSide[y], result);
		for (int x = 0; x<SCREEN_WIDTH; ++x)
		{
			vec3 color=result[x];
			PutPixelSDL(screen, x, y, color);
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
