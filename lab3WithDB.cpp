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
// STRUCTURE
struct Pixel{ int x; int y; float zinv; };
struct PixelFloat{ float x; float y; float zinv; };

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
void DrawPolygonDB(const vector<vec3>& vertices);
void VertexShader(const vec3& v, Pixel& p);
void InterpolatePixel(Pixel a, Pixel b, vector<Pixel>& result);
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);


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

	SDL_SaveBMP(screen, "screenshot3_43.png");
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
		cameraPos += vec3(0.0, -0.5, 0.0);
		;

	if (keystate[SDLK_s])
		cameraPos += vec3(0.0, 0.5, 0.0);
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

		//if ((i == 10) | (i == 13) | (i == 11) | (i == 12) | (i == 14) | (i == 15) | (i == 16) | (i == 17) | (i == 18) | (i == 19))
		if (i!=10)
		{
			DrawPolygonDB(vertices);
		}


		/*
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

void VertexShader(const vec3& v, Pixel& p)
{
	vec3 v2 = vec3((v - cameraPos)*R*R2);
	p.x = focalLength * v2[0] / v2[2] + SCREEN_WIDTH / 2;
	p.y = focalLength * v2[1] / v2[2] + SCREEN_HEIGHT / 2;
	p.zinv = 1 / v2[2];
}

/*
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
*/

void InterpolatePixel(Pixel a, Pixel b, vector<Pixel>& result)
{
	int N = result.size();
	float stepx = int(b.x - a.x) / float(max((N - 1), 1));
	float stepy = int(b.y - a.y) / float(max((N - 1), 1));
	float stepz = (b.zinv - a.zinv) / float(max((N - 1), 1));

	vector<PixelFloat> current(N);
	for (int i = 0; i<N; ++i)
	{
		current[i].x = a.x + stepx*i;
		current[i].y = a.y + stepy*i;
		current[i].zinv = a.zinv + stepz*i;
	}

	for (int i = 0; i < N; ++i)
	{
		result[i].x = int(current[i].x);
		result[i].y = int(current[i].y);
		result[i].zinv = (current[i].zinv);
	}
}


/*
void DrawLineSDL(SDL_Surface* surface, Pixel a, Pixel b, vec3 color)
{
int deltax = glm::abs(a.x - b.x);
int deltay = glm::abs(a.y - b.y);
int pixels = glm::max(deltax, deltay) + 1;
vector<Pixel> line(pixels);
InterpolatePixel(a, b, line);
for (int i = 0; i< pixels; ++i)
{
PutPixelSDL(screen, line[i].x, line[i].y, color);
}
}
*/

/*
void DrawPolygonEdges(const vector<vec3>& vertices)
{
int V = vertices.size();
// Transform each vertex from 3D world position to 2D image position:
vector<Pixel> projectedVertices(V);
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
*/


void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
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
					if (rightPixels[i].x < edge1[j].x)
						rightPixels[i].x = edge1[j].x;
					rightPixels[i].zinv = edge1[j].zinv;
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
			int row_x = row_line[j].x;
			int row_y = row_line[j].y;
			if (row_x >= 0 && row_x < 500 && row_y >= 0 && row_y < 500)
				if (row_line[j].zinv >= depthBuffer[row_y][row_x])
			    //if (1==1)
				{
					depthBuffer[row_y][row_x] = row_line[j].zinv;
					PutPixelSDL(screen, row_x, row_y, currentcolor);
				}
			//x = x + 1;
		}
	}
}

void DrawPolygonDB(const vector<vec3>& vertices)
{
	int V = vertices.size();
	vector<Pixel> vertexPixels(V);
	for (int i = 0; i<V; ++i)
	{
		VertexShader(vertices[i], vertexPixels[i]);
	}
	vector<Pixel> leftPixels(ROWS);
	vector<Pixel> rightPixels(ROWS);
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);

	DrawRows(leftPixels, rightPixels);

}
