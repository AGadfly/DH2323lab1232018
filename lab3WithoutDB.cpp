#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include <algorithm>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::ivec2;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
float focalLength = 500;
vec3 cameraPos(0, 0, -3.001);
SDL_Surface* screen;
mat3 R, R2;
double roll = 0.0;
double yaw = 0.0;
int ROWS = 1;
vec3 color(0, 0, 0);
vec3 currentcolor(0, 0, 0);
vector<ivec2> leftPixels(ROWS);
vector<ivec2> rightPixels(ROWS);
vector<ivec2> edge1, edge2, edge3;
int t;
vector<Triangle> triangles;

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
void VertexShader(const vec3& v, ivec2& p);
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color);
void DrawPolygonEdges(const vector<vec3>& vertices);
void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void DrawRows(const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels);
void DrawPolygon(const vector<vec3>& vertices);


int main(int argc, char* argv[])
{
	LoadTestModel(triangles);
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.

	while (NoQuitMessageSDL())
	{
		Update();
		Draw();
	}

	SDL_SaveBMP(screen, "screenshot3_3.png");
	return 0;
}

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
		roll = roll - 0.05;
		for (int r = 0; r < 3; r++) // row
		{
			for (int c = 0; c < 3; c++)
				R2[r][c] = 0;
		}
		R2[0][0] = 1;
		R2[1][1] = cos(roll);
		R2[1][2] = -sin(roll);
		R2[2][1] = sin(roll);
		R2[2][2] = cos(roll);
		//cameraPos = R*cameraPos;
	}

	if (keystate[SDLK_DOWN])
	{
		roll = roll + 0.05;
		for (int r = 0; r < 3; r++) // row
		{
			for (int c = 0; c < 3; c++)
				R2[r][c] = 0;
		}

		R2[0][0] = 1;
		R2[1][1] = cos(roll);
		R2[1][2] = -sin(roll);
		R2[2][1] = sin(roll);
		R2[2][2] = cos(roll);
		//cameraPos = R*cameraPos;
	}


	if (keystate[SDLK_RIGHT])
	{
		yaw = yaw - 0.05;
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

	if (keystate[SDLK_LEFT])
	{
		yaw = yaw + 0.05;
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
	if (keystate[SDLK_RSHIFT])
		;

	if (keystate[SDLK_RCTRL])
		;

	if (keystate[SDLK_w])
		;

	if (keystate[SDLK_s])
		;

	if (keystate[SDLK_d])
		;

	if (keystate[SDLK_a])
		;

	if (keystate[SDLK_e])
		;

	if (keystate[SDLK_q])
		;
}

void Draw()
{
	SDL_FillRect(screen, 0, 0);

	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);

	for (int i = 0; i < triangles.size(); ++i)
	{
		vector<vec3> vertices(3);

		vertices[0] = triangles[i].v0;
		vertices[1] = triangles[i].v1;
		vertices[2] = triangles[i].v2;
		// Add drawing
		/*
		for (int v = 0; v<3; ++v)
		{
		ivec2 projPos;
		VertexShader(vertices[v], projPos);
		vec3 color(1, 1, 1);
		PutPixelSDL(screen, projPos.x, projPos.y, color);
		}

		ivec2 a(0, 0);
		ivec2 b(500, 500);
		vec3 color(1, 1, 1);
		//Interpolate(a, b, line);
		DrawLineSDL(screen, a, b, color);
		*/

		currentcolor = triangles[i].color;
		DrawPolygon(vertices);
		//DrawPolygonEdges(vertices);

		//cout << 1 << endl;
		/*
		vector<ivec2> vertexPixels(3);
		vertexPixels[0] = ivec2(10, 5);
		vertexPixels[1] = ivec2(5, 10);
		vertexPixels[2] = ivec2(15, 15);
		vector<ivec2> leftPixels;
		vector<ivec2> rightPixels;
		ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
		for (int row = 0; row<leftPixels.size(); ++row)
		{
		cout << "Start: ("
		<< leftPixels[row].x << ","
		<< leftPixels[row].y << "). "
		<< "End: ("
		<< rightPixels[row].x << ","
		<< leftPixels[row].y << "). " << endl;
		}


		if (i == 7)
		{
		break;
		}
		*/
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void VertexShader(const vec3& v, ivec2& p)
{
	vec3 v2 = vec3((v - cameraPos)*R*R2);
	p.x = focalLength * v2[0] / v2[2] + SCREEN_WIDTH / 2;
	p.y = focalLength * v2[1] / v2[2] + SCREEN_HEIGHT / 2;
}


void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result)
{
	int N = result.size();
	vec2 step = vec2(b - a) / float(max((N - 1), 1));
	vec2 current = vec2(a);

	for (int i = 0; i<N; ++i)
	{
		result[i] = current;
		current += step;
	}
}


void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color)
{
	ivec2 delta = glm::abs(a - b);
	int pixels = glm::max(delta.x, delta.y) + 1;
	vector<ivec2> line(pixels);
	Interpolate(a, b, line);
	for (int i = 0; i< pixels; ++i)
	{
		PutPixelSDL(screen, line[i].x, line[i].y, color);
	}
}

void DrawPolygonEdges(const vector<vec3>& vertices)
{
	int V = vertices.size();
	// Transform each vertex from 3D world position to 2D image position:
	vector<ivec2> projectedVertices(V);
	for (int i = 0; i<V; ++i)
	{
		VertexShader(vertices[i], projectedVertices[i]);
	}
	// Loop over all vertices and draw the edge from it to the next vertex:
	for (int i = 0; i<V; ++i)
	{
		int j = (i + 1) % V; // The next vertex
		vec3 color(1, 1, 1);
		DrawLineSDL(screen, projectedVertices[i], projectedVertices[j], color);
	}
}

void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels)
{
	///////////////////////////
	//   find max and min y  //
	///////////////////////////
	int y_max, y_min;
	y_max = vertexPixels[0].y;
	y_min = vertexPixels[0].y;
	for (int i = 0; i < vertexPixels.size(); ++i)
	{
		if (y_max <= vertexPixels[i].y)
			y_max = vertexPixels[i].y;
		if (y_min >= vertexPixels[i].y)
			y_min = vertexPixels[i].y;
	}

	/////////////////////////////////////
	//   Resize left and right pixels  //
	/////////////////////////////////////
	int ROWS = y_max - y_min + 1;
	leftPixels.resize(ROWS);
	rightPixels.resize(ROWS);

	///////////////////////////////////////////////////
	//   Initialize with maximun and minimun number  //
	///////////////////////////////////////////////////
	for (int i = 0; i < ROWS; ++i)
	{
		leftPixels[i].x = +numeric_limits<int>::max();
		rightPixels[i].x = -numeric_limits<int>::max();
	}

	//////////////////////////////////
	// update left and right pixels //
	//////////////////////////////////
	for (int i = 0; i < ROWS; ++i)
	{
		int y = y_min + i;
		leftPixels[i].y = y;
		rightPixels[i].y = y;
	}

	for (int n = 0; n < vertexPixels.size(); n++){
		ivec2 v1 = vertexPixels[n];
		ivec2 v2 = vertexPixels[(n + 1) % vertexPixels.size()];
		int length1 = glm::abs(v1.y - v2.y) + 1;
		int length2 = glm::abs(v1.x - v2.x) + 1;
		length1 = glm::max(length1, length2);
		edge1.resize(length1);
		Interpolate(v1, v2, edge1);
		for (int i = 0; i < ROWS; i++)
		{
			int y = y_min + i;
			int y2 = 0;
			for (int j = 0; j < length1; j++)
			{
				y2 = edge1[j].y;
				if (y == y2)
				{
					leftPixels[i].x = glm::min(leftPixels[i].x, edge1[j].x);
					rightPixels[i].x = glm::max(rightPixels[i].x, edge1[j].x);
				}
			}
		}

	}

	for (int i = 0; i < ROWS; i++)
	{
		if (leftPixels[i].x >= rightPixels[i].x) //numeric_limits<int>::max())
		{
			if ((1 <= i) && (i < (ROWS - 1))){
				leftPixels[i].x = (leftPixels[i - 1].x + leftPixels[i + 1].x) / 2;
				rightPixels[i].x = (rightPixels[i - 1].x + rightPixels[i + 1].x) / 2;
			}
		}
	}
	//*/
	//int abc = 11;
}

void DrawRows(const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels)
{
	for (int i = 0; i < leftPixels.size(); ++i)
	{
		int length = rightPixels[i].x - leftPixels[i].x + 1;
		int x = leftPixels[i].x;
		int y = leftPixels[i].y;
		for (int j = 0; j < length; j++)
		{
			PutPixelSDL(screen, x, y, currentcolor);
			x = x + 1;
		}
	}
}

void DrawPolygon(const vector<vec3>& vertices)
{
	int V = vertices.size();
	vector<ivec2> vertexPixels(V);
	for (int i = 0; i<V; ++i)
	{
		VertexShader(vertices[i], vertexPixels[i]);
	}
	vector<ivec2> leftPixels;
	vector<ivec2> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	DrawRows(leftPixels, rightPixels);
}
