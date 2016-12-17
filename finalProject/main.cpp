#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "assignment1.app/Contents/Resources/"
#endif

#define PI 3.141592653589793238

SDL_Window* displayWindow;


class Vector3 {
public:
	Vector3() { Vector3(0, 0, 0); }
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;

};

class SheetSprite {
public:
	SheetSprite() { SheetSprite(0.0f, 0); }
	SheetSprite(float size, unsigned int textureID, float u = 0, float v = 0, float width = 0, float height = 0) : size(size), textureID(textureID), u(u), v(v), width(width), height(height) {}

	void Draw(ShaderProgram *program) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspect = width / height;
		float vertices[] = {
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size ,
			0.5f * size * aspect, -0.5f * size };
		// draw our arrays
		glBindTexture(GL_TEXTURE_2D, textureID);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

class Entity {
public:
	//Entity() { Entity(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f, SheetSprite()); }
	//Entity(Vector3 position, Vector3 velocity, Vector3 size, float rotation, SheetSprite sprite) : position(position), velocity(velocity), size(size), rotation(rotation), sprite(sprite) {}
	Entity(std::string type, bool isStatic, Vector3 position, Vector3 velocity = Vector3(0, 0, 0), Vector3 acceleration = Vector3(0, 0, 0), Vector3 size = Vector3(0, 0, 0), float rotation = 0.0f) :
		type(type), isStatic(isStatic), position(position), velocity(velocity), acceleration(acceleration), size(size), rotation(rotation), exists(true), collidedTop(false), collidedBottom(false), collidedLeft(false), collidedRight(false) {}
	//void Draw();
	std::string type;
	bool exists;
	bool isStatic;
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 size;
	float rotation;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
	//SheetSprite sprite;
};


GLuint LoadTexture(const char *image_path) 
{
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

float TILE_SIZE = .2;

void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX, int spriteCountY, GLuint textureID) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;

	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};

	float vertices[] = { -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE };
	
	glBindTexture(GL_TEXTURE_2D, textureID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int mapWidth;
int mapHeight;
unsigned char** levelData;

bool readHeader(std::ifstream &stream) {
	std::string line;
	mapWidth = -1;
	mapHeight = -1;
	while
		(getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height") {
			mapHeight = atoi(value.c_str());
		}
	}
	if
		(mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool readLayerData(std::ifstream &stream) {
	std::string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				std::istringstream lineStream(line);
				std::string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

std::vector<Entity> entities;
int pIndex;

void placeEntity(std::string type, float x, float y) {
	bool isStatic = true;
	if (type == "player" || type == "crab") {
		isStatic = false;
	}
	if (type == "player") {
		pIndex = entities.size();
	}
	Entity entity(type, isStatic, Vector3(x, y, 0));
	entities.push_back(entity);
}

bool readEntityData(std::ifstream &stream) {
	std::string line;
	std::string type;
	//OutputDebugStringA("---------attempting to read entities\n");
	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			//OutputDebugStringA("0\n");
			std::istringstream lineStream(value);
			std::string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str())* TILE_SIZE;
			float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}

float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-(worldY) / TILE_SIZE);
}


void collisionx(Entity &entity) {
	int gridX, gridY;

	//check left
	worldToTileCoordinates(entity.position.x, entity.position.y + 0.008, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1 * (entity.position.x) - gridX * TILE_SIZE - TILE_SIZE + 0.008;
		entity.velocity.x = 0;
		entity.acceleration.x = 0;
		entity.collidedLeft = true;
	}
	worldToTileCoordinates(entity.position.x, entity.position.y + TILE_SIZE - 0.008, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1 * (entity.position.x) - gridX * TILE_SIZE - TILE_SIZE + 0.008;
		entity.velocity.x = 0;
		entity.acceleration.x = 0;
		entity.collidedLeft = true;
	}
	//check right
	worldToTileCoordinates(entity.position.x + TILE_SIZE, entity.position.y + 0.008, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1 * (entity.position.x) - gridX * TILE_SIZE + TILE_SIZE - 0.008;
		entity.velocity.x = 0;
		entity.acceleration.x = 0;
		entity.collidedRight = true;
	}
	worldToTileCoordinates(entity.position.x + TILE_SIZE, entity.position.y + TILE_SIZE - 0.008, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1 * (entity.position.x) - gridX * TILE_SIZE + TILE_SIZE - 0.008;
		entity.velocity.x = 0;
		entity.acceleration.x = 0;
		entity.collidedRight = true;
	}
}

void collisiony(Entity &entity) {
	int gridX, gridY;
	float top = 0;
	if (entity.type == "player") {
		top = TILE_SIZE;
	}
	//check bottom
	worldToTileCoordinates(entity.position.x + 0.008, entity.position.y, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		entity.position.y += -1 * (entity.position.y) - gridY * TILE_SIZE + 0.001;
		entity.velocity.y = 0;
		entity.collidedBottom = true;
	}
	worldToTileCoordinates(entity.position.x + TILE_SIZE - 0.008, entity.position.y, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		entity.position.y += -1 * (entity.position.y) - gridY * TILE_SIZE + 0.001;
		entity.velocity.y = 0;
		entity.collidedBottom = true;
	}
	//check top
	worldToTileCoordinates(entity.position.x + 0.008, entity.position.y + TILE_SIZE + top, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		entity.position.y += -1 * (entity.position.y + TILE_SIZE + top) - gridY * TILE_SIZE - TILE_SIZE - 0.001;
		entity.velocity.y = 0;
		entity.collidedTop = true;
	}
	worldToTileCoordinates(entity.position.x + TILE_SIZE - 0.008, entity.position.y + TILE_SIZE + top, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		entity.position.y += -1 * (entity.position.y + TILE_SIZE + top) - gridY * TILE_SIZE - TILE_SIZE - 0.001;
		entity.velocity.y = 0;
		entity.collidedTop = true;
	}
}

bool entityCollision(Entity e1, Entity e2) {
	int top = 0;
	if (e1.type == "player") {
		top = TILE_SIZE;
	}
	return (!(
		e1.position.x - TILE_SIZE / 2 > e2.position.x + TILE_SIZE / 2 ||
		e1.position.x + TILE_SIZE / 2 < e2.position.x - TILE_SIZE / 2 ||
		e1.position.y - TILE_SIZE / 2 > e2.position.y + TILE_SIZE / 2 ||
		e1.position.y + TILE_SIZE / 2 + top < e2.position.y - TILE_SIZE / 2));
}

float distance(double x1, double y1, double x2, double y2) {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}


#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
void Update(float p1ax, float p1vy, float ticks, float time) {
	for (int i = 0; i < entities.size(); ++i) {
		if (entities[i].type == "player") {
			entities[i].acceleration.x = p1ax;
			if (entities[i].collidedBottom) {
				entities[i].velocity.y = p1vy;
			}
		}
		else if (entities[i].type == "gear") {
			entities[i].position.y += 0.001 * sin(ticks * 10 / PI);
		}
		if (!entities[i].isStatic) {
			entities[i].acceleration.y = -5;
		}
		entities[i].collidedTop = false;
		entities[i].collidedBottom = false;
		entities[i].collidedLeft = false;
		entities[i].collidedRight = false;

		entities[i].velocity.x = lerp(entities[i].velocity.x, 0.0f, time * 5);
		entities[i].velocity.y = lerp(entities[i].velocity.y, 0.0f, time * 1);

		entities[i].velocity.x += entities[i].acceleration.x * time;
		entities[i].velocity.y += entities[i].acceleration.y * time;

		entities[i].position.y += entities[i].velocity.y * time;
		if (!entities[i].isStatic) {
			collisiony(entities[i]);
		}
		entities[i].position.x += entities[i].velocity.x * time;
		if (!entities[i].isStatic) {
			collisionx(entities[i]);
		}
	}
}

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

enum letterIndex { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z };
int LETTER_SHIFT = 65;

enum GameState { STATE_MAIN_MENU, STATE_CONTROLS, STATE_GAME_LEVEL, STATE_GAME_OVER };
int state = STATE_MAIN_MENU;

SDL_Event event;
bool done = false;

void main_menu(ShaderProgram &program, GLuint &letters) {

	int playGameArr[] = { P, L, A, Y, -1, G, A, M, E };
	int controlsArr[] = { C, O, N, T, R, O, L, S };
	int exitArr[] = { E, X, I, T };

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN) { // MOUSEBUTTONDOWN
													  // Convert from pixels to OpenGL units
													  // units_x = (pixel_x / x_resolution) * ortho_width ) - ortho_width / 2.0;
			float units_x = (((float)event.motion.x / 1280) * 7.1f) - 3.55f;
			// units_y = ((y_resolution - pixel_y) / y_resolution) * ortho_height) - ortho_height / 2.0;
			float units_y = (((float)(720 - event.motion.y) / 720) * 4.0f) - 2.0f;

			if (units_x > -2.1f && units_x < -0.4f && units_y > -0.1f && units_y < 0.1f) {
				state = STATE_GAME_LEVEL;
			}

			if (units_x > -2.1f && units_x < -0.6f && units_y > -0.6f && units_y < -0.4f) {
				state = STATE_CONTROLS;
				//state = STATE_GAME_LEVEL;
			}

			if (units_x > -2.1f && units_x < -1.4f && units_y > -1.1f && units_y < -0.9f) {
				/*SDL_Quit();
				return 0;*/
				done = true;
			}
		}
	}
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < 9; ++i) { // Length of playGameArr
		if (playGameArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45 + i*.2 + 1.4, 0, 0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, playGameArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 8; ++i) { // Length of controlsArr
		if (controlsArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45 + i*.2 + 1.4, -0.5, 0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, controlsArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 4; ++i) { // Length of exitArr
		if (exitArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45 + i*.2 + 1.4, -1, 0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, exitArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	SDL_GL_SwapWindow(displayWindow);
}

void controls(ShaderProgram &program, GLuint &controlsPage) {

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				state = STATE_MAIN_MENU;
			}
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);

	program.setModelMatrix(modelMatrix);
	program.setProjectionMatrix(projectionMatrix);
	program.setViewMatrix(viewMatrix);

	modelMatrix.identity();

	//GLuint controlsPage = LoadTexture("controls.png");

	glBindTexture(GL_TEXTURE_2D, controlsPage);

	float vertices[] = { -3.55, -2.0, 3.55, -2.0, 3.55, 2.0, -3.55, -2.0, 3.55, 2.0, -3.55, 2.0 };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	SDL_GL_SwapWindow(displayWindow);
}

void game_over(ShaderProgram &program, GLuint &gameOverPage, GLuint &letters) {

	int playAgainArr[] = { P, L, A, Y, -1, A, G, A, I, N };
	int exitArr[] = { E, X, I, T };

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN) { // MOUSEBUTTONDOWN
												  // Convert from pixels to OpenGL units
												  // units_x = (pixel_x / x_resolution) * ortho_width ) - ortho_width / 2.0;
			float units_x = (((float)event.motion.x / 1280) * 7.1f) - 3.55f;
			// units_y = ((y_resolution - pixel_y) / y_resolution) * ortho_height) - ortho_height / 2.0;
			float units_y = (((float)(720 - event.motion.y) / 720) * 4.0f) - 2.0f;

			if (units_x > -1.0f && units_x < 1.0f && units_y > -0.9f && units_y < -0.7f) {
				state = STATE_GAME_LEVEL;
			}

			if (units_x > -0.4f && units_x < 0.3f && units_y > -1.4f && units_y < -1.2f) {
				done = true;
			}
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);

	// Draw background image
	program.setModelMatrix(modelMatrix);
	program.setProjectionMatrix(projectionMatrix);
	program.setViewMatrix(viewMatrix);

	//GLuint gameOverPage = LoadTexture("game_over.png");

	glBindTexture(GL_TEXTURE_2D, gameOverPage);

	float vertices[] = { -3.55, -2.0, 3.55, -2.0, 3.55, 2.0, -3.55, -2.0, 3.55, 2.0, -3.55, 2.0 };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	// Draw playGame and exit buttons
	for (int i = 0; i < 10; ++i) { // Length of playAgainArr
		if (playAgainArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-0.9 + i*.2, -0.8, 0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, playAgainArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 4; ++i) { // Length of exitArr
		if (exitArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-0.3 + i*.2, -1.3, 0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, exitArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	modelMatrix.identity();

	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	//displayWindow = SDL_CreateWindow("MAGNETIC MAN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	displayWindow = SDL_CreateWindow("MAGNETIC MAN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	//glViewport(0, 0, 640, 360);
	glViewport(0, 0, 1280, 720);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	float lastFrameTicks = 0.0f;
	
	GLuint controlsPage = LoadTexture("controls.png");
	GLuint gameOverPage = LoadTexture("game_over.png");
	GLuint letters = LoadTexture("letters.png");
	GLuint goldenGearSpriteSheet = LoadTexture("golden_gear_spritesheet.png");
	//GLuint spriteSheet = LoadTexture("spritesheet.png");

	std::ifstream infile("world1.txt");
	std::string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return 0;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[objectLayer]") {
			readEntityData(infile);
		}
	}

	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	float p1vy = 0;
	float p1ax = 0.0f;

	while (!done) {

		switch (state) {

		case STATE_MAIN_MENU:
			main_menu(program, letters);
			break;

		case STATE_CONTROLS:
			//controls(program, controlsPage);
			game_over(program, gameOverPage, letters);
			break;

		case STATE_GAME_OVER:
			game_over(program, gameOverPage, letters);

		case STATE_GAME_LEVEL:
			p1ax = 0;
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_K]) {
				p1ax += -5;
			}
			if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_SEMICOLON]) {
				p1ax += 5;
			}
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1vy = 3.1;
					}
				}
				else if (event.type == SDL_KEYUP) {
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1vy = 0;
					}
				}
				if (event.type == SDL_MOUSEBUTTONDOWN) {
					if (event.button.button == 1) {
						
					}
				}
				else if (event.type == SDL_MOUSEBUTTONUP) {
					if (event.button.button == 1) {
						
					}
				}
			}

			////////MOVEMENT//////////////////////////////////////////////////////////////////////////////////
			//timestep
			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			// 60 FPS (1.0f/60.0f)
			float fixedElapsed = elapsed;
			if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
				fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
			}
			while (fixedElapsed >= FIXED_TIMESTEP) {
				fixedElapsed -= FIXED_TIMESTEP;
				Update(p1ax, p1vy, ticks, FIXED_TIMESTEP);
			}
			Update(p1ax, p1vy, ticks, fixedElapsed);

			////////ENTITY COLLISION//////////////////////////////////////////////////////////////////////////
			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].type == "player") {
					for (int j = 0; j < entities.size(); ++j) {
						if (entities[j].type == "gear" && entityCollision(entities[i], entities[j])) {
							//entities.erase(entities.begin() + j);
							entities[j].exists = false;
							break;
						}
					}
					break;
				}
			}

			///////DRAWING////////////////////////////////////////////////////////////////////////////////////
			glClear(GL_COLOR_BUFFER_BIT);

			double minDist = 1000000000;
			int minx = -1;
			int miny = -1;

			for (int y = 0; y < mapHeight; ++y) {
				for (int x = 0; x < mapWidth; ++x) {
					modelMatrix.identity();
					//modelMatrix.Translate(x*TILE_SIZE - (3.55 - TILE_SIZE/2),(32-y - 1)*TILE_SIZE - (2.0f - TILE_SIZE/2), 0);
					modelMatrix.Translate(x*TILE_SIZE, (mapHeight - y - 1)*TILE_SIZE, 0);
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);
					DrawSpriteSheetSprite(&program, levelData[y][x], 20, 10, goldenGearSpriteSheet);
					
					int mx, my;

					float units_x = (((float)event.motion.x / 1280) * 7.1f) - 3.55f + entities[pIndex].position.x + TILE_SIZE / 2;
					float units_y = (((float)(720 - event.motion.y) / 720) * 4.0f) - 2.0f + entities[pIndex].position.y + mapHeight*TILE_SIZE + TILE_SIZE / 2;
					worldToTileCoordinates(units_x, units_y, &mx, &my);
					my = my + mapHeight - 1;

					if (levelData[y][x] >= 9 && levelData[y][x] <= 12 && distance(mx, my, x, y) < minDist) {
						minDist = distance(mx, my, x, y);
						minx = x;
						miny = y;					
					}
					/*
					if (levelData[y][x] >= 9 && levelData[y][x] <= 12 && mx == x && my == y) {
						modelMatrix.identity();
						//modelMatrix.Translate(x*TILE_SIZE - (3.55 - TILE_SIZE/2),(32-y - 1)*TILE_SIZE - (2.0f - TILE_SIZE/2), 0);
						modelMatrix.Translate(x*TILE_SIZE, (mapHeight - y - 1)*TILE_SIZE, 0);
						program.setModelMatrix(modelMatrix);
						program.setProjectionMatrix(projectionMatrix);
						program.setViewMatrix(viewMatrix);
						DrawSpriteSheetSprite(&program, 27, 20, 10, goldenGearSpriteSheet);
					}*/

				}
			}
			if (minx >= 0 && miny >= 0) {
				modelMatrix.identity();
				modelMatrix.Translate(minx*TILE_SIZE, (mapHeight - miny - 1)*TILE_SIZE, 0);
				program.setModelMatrix(modelMatrix);
				program.setProjectionMatrix(projectionMatrix);
				program.setViewMatrix(viewMatrix);
				DrawSpriteSheetSprite(&program, 27, 20, 10, goldenGearSpriteSheet);
			}
			/*
			int mx;
			int my;
			float units_x = (((float)event.motion.x / 1280) * 7.1f) - 3.55f;
			float units_y = (((float)(720 - event.motion.y) / 720) * 4.0f) - 2.0f;
			worldToTileCoordinates(units_x, units_y, &mx, &my);

			modelMatrix.identity();
			//modelMatrix.Translate(x*TILE_SIZE - (3.55 - TILE_SIZE/2),(32-y - 1)*TILE_SIZE - (2.0f - TILE_SIZE/2), 0);
			modelMatrix.Translate(mx*TILE_SIZE, (mapHeight - my - 1)*TILE_SIZE, 0);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, 27, 20, 10, goldenGearSpriteSheet);
			*/
			std::vector<std::string> types = {"gear", "silverGear", "goldenGear", "target", "crab", "star", "bullet"};
			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].exists) {
					modelMatrix.identity();
					//modelMatrix.Translate(entities[i].position.x - 3.55 + TILE_SIZE / 2, entities[i].position.y + TILE_SIZE / 2 + (mapHeight*TILE_SIZE - 2.0f), 0);
					modelMatrix.Translate(entities[i].position.x, entities[i].position.y + mapHeight*TILE_SIZE, 0);
					if (entities[i].type == "gear" || entities[i].type == "silverGear" || entities[i].type == "target") {
						modelMatrix.Rotate(-1 * ticks);
					}
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);
					if (entities[i].type == "player") {
						DrawSpriteSheetSprite(&program, 60, 20, 10, goldenGearSpriteSheet);

						modelMatrix.identity();
						//modelMatrix.Translate(entities[i].position.x - 3.55 + TILE_SIZE / 2, entities[i].position.y + 3 * TILE_SIZE / 2 + (mapHeight*TILE_SIZE - 2.0f), 0);
						modelMatrix.Translate(entities[i].position.x, entities[i].position.y + mapHeight*TILE_SIZE + TILE_SIZE, 0);

						program.setModelMatrix(modelMatrix);
						program.setProjectionMatrix(projectionMatrix);
						program.setViewMatrix(viewMatrix);
						DrawSpriteSheetSprite(&program, 40, 20, 10, goldenGearSpriteSheet);

						viewMatrix.identity();
						viewMatrix.Translate(-entities[i].position.x, -1 * (entities[i].position.y + mapHeight*TILE_SIZE), 0);
					}
					else {
						int pos = find(types.begin(), types.end(), entities[i].type) - types.begin();
						DrawSpriteSheetSprite(&program, pos + 24, 20, 10, goldenGearSpriteSheet);
					}
				}
			}			
			SDL_GL_SwapWindow(displayWindow);
			break;
		}
	}

	SDL_Quit();
	return 0;
}