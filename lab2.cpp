#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream


using namespace std;
using glm::vec3;
using glm::mat3;
//////////////////////////////////////////////
//             GLOBAL VARIABLES             //
//////////////////////////////////////////////
const int SCREEN_WIDTH  = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;
float step = 0.5;           // camera movement step
vector<Triangle> triangles; // used for triangles model storage
vec3 cameraPos(0, 0, -2);
vec3 cameraPos_update;
float focalLength = float(SCREEN_HEIGHT / 2);
mat3 R;
float yaw = 0.0f;
float roll = 0.0f;
vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f * vec3(1, 1, 1);
vec3 indirectLight = 0.5f*vec3(1, 1, 1);
//////////////////////////////////////////////
//                Structure                 //
//////////////////////////////////////////////
struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};
Intersection closestIntersection; // used for saving closest triangle color

struct photon {
	float x, y, z; // position
	char p[4]; // power packed as 4 chars
	char phi, theta; // compressed incident direction
	short flag; // flag used in kdtree
};
//////////////////////////////////////////////
//                 Function                 //
//////////////////////////////////////////////
void Update();
void Draw();
bool ClosestIntersection(
	vec3 start,
	vec3 dir,
	const vector<Triangle>& triangles,
	Intersection& closestIntersection
	);
vec3 DirectLight(const Intersection& i);
void Left_right();
void Up_down();
//////////////////////////////////////////////
//              Main Function               //
//////////////////////////////////////////////
int main(int argc, char* argv[])
{
	LoadTestModel(triangles); // load model into triangles
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.
	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}

	SDL_SaveBMP(screen, "screenshot.bmp");
	return 0;
}
//////////////////////////////////////////////
//        Update camera position            //
//////////////////////////////////////////////
/*
void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
}
*/
void Update()
{
	// Compute frame time:
	
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
	Uint8* keystate = SDL_GetKeyState(0);
	if (keystate[SDLK_UP])
	{
		roll += 0.1f;
		Up_down();
	}
	if (keystate[SDLK_DOWN])
	{
		roll -= 0.2f;
		Up_down();
	}
	if (keystate[SDLK_LEFT])
	{
		yaw -= 0.2f;
		Left_right();
		
	}
	if (keystate[SDLK_RIGHT])
	{
		yaw += 0.1f;
		Left_right();
	}
}
//////////////////////////////////////////////
//        Fill in screen with colors        //
//////////////////////////////////////////////
void Draw()
{
	
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	
	for (int y = 0; y<SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x<SCREEN_WIDTH; ++x)
		{
			vec3 color(1, 0.5, 0.5);
			cameraPos_update = R*cameraPos;
			vec3 start = cameraPos_update;
			vec3 dir (x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2, focalLength);
			bool inte_flag;
			inte_flag = ClosestIntersection(start, dir, triangles, closestIntersection);
			if (inte_flag == true)
			{
				vec3 direct_light = DirectLight(closestIntersection);
				vec3 indirectLight = 0.5f*vec3(1, 1, 1);
				color = (direct_light + indirectLight)*triangles[closestIntersection.triangleIndex].color;
				
				PutPixelSDL(screen, x, y, color);
			}
			else
			{
				vec3 color(0, 0, 0);
				PutPixelSDL(screen, x, y, color);
			}
			
			//color = vec3(1,1,1);
			//PutPixelSDL(screen, x, y, color);
		}
	}

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
//////////////////////////////////////////////
//       choose the closest triangle        //
//              save its data               //
//////////////////////////////////////////////
bool ClosestIntersection(
	vec3 start,
	vec3 dir,
	const vector<Triangle>& triangles,
	Intersection& closestIntersection
	)
{
	bool inter_flag = false;
	int index_temp = 0;
	float dist_temp = std::numeric_limits<float>::max();
	int tria_size = triangles.size();
	vec3 post_temp(0, 0, 0);
	for (int tria_numb = 0; tria_numb < tria_size; ++tria_numb)
	{
		using glm::vec3;
		using glm::mat3;
		Triangle triangle = triangles[tria_numb];
		vec3 v0 = triangle.v0;
		vec3 v1 = triangle.v1;
		vec3 v2 = triangle.v2;
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;
		vec3 b = start - v0;
		mat3 A(-dir, e1, e2);
		vec3 x = glm::inverse(A) * b;
		if ((x[0] >= 0) && (x[1] >= 0) && (x[2] >= 0) && (x[1] + x[2] <= 1))
		{
			if (x[0] < dist_temp)
			{
				dist_temp = x[0];
				index_temp = tria_numb;
				post_temp = start + dist_temp * dir;
				inter_flag = true;
			}
		}
	}
	if (inter_flag == true)
	{
		closestIntersection.position = post_temp;
		closestIntersection.distance = dist_temp;
		closestIntersection.triangleIndex = index_temp;

		//std::cout << index_temp << '\n';
	}
	return inter_flag;
}
//////////////////////////////////////////////
//            Direct line Function          //
//////////////////////////////////////////////
vec3 DirectLight(const Intersection& i)
{
	vec3 norm, r, direct_light_color, multi;
	float pi = 3.1415;
	Intersection shadow;
	bool shad = false;
	Triangle tria = triangles[i.triangleIndex];
	norm = tria.normal;
	r = lightPos - i.position;
	multi = norm * r;
	float cos_theta = multi[0] + multi[1] + multi[2];
	if (cos_theta > 0.0f)
	{
		float length = sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
		direct_light_color = lightColor* cos_theta / (4*pi*length*length);
	}
	shad = ClosestIntersection(lightPos, -glm::normalize(r), triangles, shadow);
	if ((shadow.triangleIndex != i.triangleIndex) && (shadow.distance < glm::length(r) - 0.0001f))
	{
		direct_light_color = vec3(0.0f, 0.0f, 0.0f);
	}

	return direct_light_color;
}

//////////////////////////////////////////////
// change camera position according to yaw  //
//           rotate along y-axis            //
//////////////////////////////////////////////
void Left_right()
{
	for (int r = 0; r < 3; r++) // row
	{
		for (int c = 0; c < 3; c++)
			R[r][c] = 0;
	}
	R[0][0] = cos(yaw);
	R[0][2] = sin(yaw);
	R[2][0] = -sin(yaw);
	R[2][2] = cos(yaw);
	R[1][1] = 1;
	//cameraPos = R*cameraPos;
}

//////////////////////////////////////////////
// change camera position according to roll //
//           rotate along x-axis            //
//////////////////////////////////////////////
void Up_down()
{
	for (int r = 0; r < 3; r++) // row
	{
		for (int c = 0; c < 3; c++)
			R[r][c] = 0;
	}
	R[0][0] = 1;
	R[1][1] = cos(roll);
	R[1][2] = -sin(roll);
	R[2][1] = sin(roll);
	R[2][2] = cos(roll);
	//cameraPos = R*cameraPos;
}





