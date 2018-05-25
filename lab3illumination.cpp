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
using glm::dot;

// ----------------------------------------------------------------------------
// STRUCTURE
struct Pixel { int x; int y; float zinv; vec3 illumination; };
struct PixelFloat { float x; float y; float zinv; vec3 illumination; };
struct Vertex {vec3 position; vec3 normal; vec2 reflectance;};

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
vec3 norm(0, 0, 0);
//------LIGHT ---------//
vec3 currentcolor(0, 0, 0);
vec3 lightPos(0, -0.5, -0.7);
vec3 lightPower = 14.0f*vec3(1, 1, 1);
vec3 indirectLightPowerPerArea = 0.5f*vec3(1, 1, 1);
vector<Pixel> leftPixels(ROWS);
vector<Pixel> rightPixels(ROWS);
vector<Pixel> edge1, edge2, edge3;
int t;
vector<Triangle> triangles;
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

//void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color);
//void DrawPolygonEdges(const vector<vec3>& vertices);

//void VertexShader(const vec3& v, ivec2& p);
//void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
//void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void DrawRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels);
void DrawPolygonDB(const vector<vec3>& vertices, vec3 norm);
void VertexShader(const vec3& v, Pixel& p);
void InterpolatePixel(Pixel a, Pixel b, vector<Pixel>& result);
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vec3 norm, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
void PixelShader(const Pixel& p);

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

	SDL_SaveBMP(screen, "screenshot3_5.png");
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

	for (int y = 0; y<SCREEN_HEIGHT; ++y)
		for (int x = 0; x<SCREEN_WIDTH; ++x)
			depthBuffer[y][x] = 0;

	for (int i = 0; i < triangles.size(); ++i)
	{
		vector<vec3> vertices(3);

		vertices[0] = triangles[i].v0;
		vertices[1] = triangles[i].v1;
		vertices[2] = triangles[i].v2;
		// Add drawing
		currentcolor = triangles[i].color;
		norm = triangles[i].normal;
		//if ((i == 10) | (i == 13) | (i == 11) | (i == 12) | (i == 14) | (i == 15) | (i == 16) | (i == 17) | (i == 18) | (i == 19))
		//if ((i <20)&&(i!=11) )
		if (1==1)
		{
			DrawPolygonDB(vertices, norm);
		}
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

void VertexShader(const vec3& v, vec3 norm, Pixel& p)
{
	vec3 v2 = vec3((v - cameraPos)*R*R2);
	p.x = focalLength * v2[0] / v2[2] + SCREEN_WIDTH / 2;
	p.y = focalLength * v2[1] / v2[2] + SCREEN_HEIGHT / 2;
	p.zinv = 1 / v2[2];
	float dist1 = glm::dot((lightPos - v), norm);
	float r = glm::length(lightPos - v);
	vec3 D, R;
	if (dist1 > 0.0)
		D = dist1*lightPower / (4 * 3.1415926f * r * r);
	else
	{
		D = vec3(0.0, 0.0, 0.0);
	}
	R = currentcolor*(D + indirectLightPowerPerArea);
	p.illumination = R;
}

void PixelShader(const Pixel& p)
{
	int x = p.x;
	int y = p.y;
	if (p.zinv > depthBuffer[y][x])
	{
		depthBuffer[y][x] = p.zinv;
		PutPixelSDL(screen, x, y, p.illumination);
	}
}

void InterpolatePixel(Pixel a, Pixel b, vector<Pixel>& result)
{
	int N = result.size();
	float stepx = int(b.x - a.x) / float(max((N - 1), 1));
	float stepy = int(b.y - a.y) / float(max((N - 1), 1));
	float stepz = (b.zinv - a.zinv) / float(max((N - 1), 1));
	vec3 step_ill = vec3(b.illumination / float(max((N - 1), 1)) - a.illumination / float(max((N - 1), 1)));
	vector<PixelFloat> current(N);
	for (int i = 0; i<N; ++i)
	{
		current[i].x = a.x + stepx*i;
		current[i].y = a.y + stepy*i;
		current[i].zinv = a.zinv + stepz*i;
		current[i].illumination[0] = a.illumination[0] + step_ill[0] * i;
		current[i].illumination[1] = a.illumination[1] + step_ill[1] * i;
		current[i].illumination[2] = a.illumination[2] + step_ill[2] * i;
	}

	for (int i = 0; i < N; ++i)
	{
		result[i].x = int(current[i].x);
		result[i].y = int(current[i].y);
		result[i].zinv = current[i].zinv;
		result[i].illumination = current[i].illumination;
	}
}


void ComputePolygonRows(const vector<Pixel>& vertexPixels, vec3 norm, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
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
		Pixel v1 = vertexPixels[n];
		Pixel v2 = vertexPixels[(n + 1) % vertexPixels.size()];
		
		int length1 = glm::abs(v1.y - v2.y) + 1;
		int length2 = glm::abs(v1.x - v2.x) + 1;
		length1 = glm::max(length1, length2);
		edge1.resize(length1);
		InterpolatePixel(v1, v2, edge1);
		if (n == 2){
			n = n;
			edge1 = edge1;
		}
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
					if (leftPixels[i].x > edge1[j].x)
						leftPixels[i].x = edge1[j].x;
						leftPixels[i].zinv = edge1[j].zinv;
						leftPixels[i].illumination = edge1[j].illumination;
					if (rightPixels[i].x < edge1[j].x)
						rightPixels[i].x = edge1[j].x;
						rightPixels[i].illumination = edge1[j].illumination;
				}
			}
		}

	}
	
	for (int i = 0; i < ROWS; i++)
	{
		if (leftPixels[i].x >= rightPixels[i].x) //numeric_limits<int>::max())
		{
			if ((1 <= i) && (i < (ROWS - 1))){
				//leftPixels[i].x = (leftPixels[i - 1].x + leftPixels[i - 1].x - leftPixels[i - 2].x) / 1;
				//rightPixels[i].x = (rightPixels[i - 1].x + rightPixels[i - 1].x - rightPixels[i - 2].x) / 1;
				leftPixels[i].x = (leftPixels[i - 1].x + leftPixels[i + 1].x) / 2;
				rightPixels[i].x = (rightPixels[i - 1].x + rightPixels[i + 1].x) / 2;
			}
		}
	}
	//int abc = 11;
}

void DrawRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels)
{
	for (int i = 0; i < leftPixels.size(); ++i)
	{
		
		int length = glm::abs(rightPixels[i].x - leftPixels[i].x) + 1;
		
		//int y = leftPixels[i].y;
		vector<Pixel> row_line(length);
		InterpolatePixel(leftPixels[i], rightPixels[i], row_line);
		for (int j = 0; j < length; j++)
		{	
			PixelShader(row_line[j]);
			/*
			int row_x = row_line[j].x;
			int row_y = row_line[j].y;
			if (row_x >= 0 && row_x < 500 && row_y >= 0 && row_y < 500)
			
			//if (row_line[j].zinv > depthBuffer[row_y][row_x])
			// if (1==1)
			{
			depthBuffer[row_y][row_x] = row_line[j].zinv;
			PutPixelSDL(screen, row_x, row_y, row_line[j].illumination);
			}
			*/
			
			//x = x + 1;
		}
	}
}

void DrawPolygonDB(const vector<vec3>& vertices, vec3 norm)
{
	int V = vertices.size();
	vector<Pixel> vertexPixels(V);
	for (int i = 0; i<V; ++i)
	{
		VertexShader(vertices[i], norm, vertexPixels[i]);
	}
	vector<Pixel> leftPixels(ROWS);
	vector<Pixel> rightPixels(ROWS);
	ComputePolygonRows(vertexPixels, norm, leftPixels, rightPixels);
	DrawRows(leftPixels, rightPixels);
}
