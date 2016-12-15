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
		type(type), isStatic(isStatic), position(position), velocity(velocity), acceleration(acceleration), size(size), rotation(rotation), collidedTop(false), collidedBottom(false), collidedLeft(false), collidedRight(false) {}
	//void Draw();
	std::string type;
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

void placeEntity(std::string type, float x, float y) {
	bool isStatic = true;
	if (type == "player" || type == "crab") {
		isStatic = false;
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
	
	//GLuint spriteSheet = LoadTexture("spritesheet.png");
	GLuint letters = LoadTexture("letters.png");
	GLuint goldenGearSpriteSheet = LoadTexture("golden_gear_spritesheet.png");
	enum letterIndex { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z };
	int LETTER_SHIFT = 65;
	int message[] = { P, R, E, S, S, -1, S, P, A, C, E, -1, T, O, -1, B, E, G, I, N };

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

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);


	float p1x = 0;
	float p1vy = 0;
	bool p1shoot = false;
	int bulletCooldown = 0;
	float espurrx = 0;
	float espurrSpeed = 0.05;
	int inactiveBullet = 6; //current inactive bullet index
	enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
	int state = STATE_MAIN_MENU;
	float p1ax = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {

		switch (state) {

		case STATE_MAIN_MENU:
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						state = STATE_GAME_LEVEL;
					}
				}
			}
			glClear(GL_COLOR_BUFFER_BIT);
			
			for (int i = 0; i < 20; ++i) {
				if (message[i] != -1) {
					modelMatrix.identity();
					modelMatrix.Translate(-3.45 + i*.2 + 1.4, 0, 0);
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);
					DrawSpriteSheetSprite(&program, message[i] + 65, 16, 16, letters);
				}
			}
			SDL_GL_SwapWindow(displayWindow);
			break;

		case STATE_GAME_LEVEL:
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT || event.key.keysym.scancode == SDL_SCANCODE_SEMICOLON || event.key.keysym.scancode == SDL_SCANCODE_D) {
						p1ax = 5;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_K || event.key.keysym.scancode == SDL_SCANCODE_A) {
						p1ax = -5;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1vy = 3.1;
					}
				}
				else if (event.type == SDL_KEYUP) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT || event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_SEMICOLON || 
						event.key.keysym.scancode == SDL_SCANCODE_D || event.key.keysym.scancode == SDL_SCANCODE_K || event.key.keysym.scancode == SDL_SCANCODE_A) {
						p1ax = 0;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1vy = 0;
					}
				}
				if (event.type == SDL_MOUSEBUTTONDOWN && bulletCooldown <= 0) {
					if (event.button.button == 1) {
						p1shoot = true;
					}
				}
				else if (event.type == SDL_MOUSEBUTTONUP) {
					if (event.button.button == 1) {
						p1shoot = false;
					}
				}
			}

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

			////////MOVEMENT//////////////////////////////////////////////////////////////////////////////////
			/*for (int i = 0; i < entities.size(); ++i) {
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

				entities[i].velocity.x = lerp(entities[i].velocity.x, 0.0f, FIXED_TIMESTEP * 5);
				entities[i].velocity.y = lerp(entities[i].velocity.y, 0.0f, FIXED_TIMESTEP * 1);

				entities[i].velocity.x += entities[i].acceleration.x * FIXED_TIMESTEP;
				entities[i].velocity.y += entities[i].acceleration.y * FIXED_TIMESTEP;

				entities[i].position.x += entities[i].velocity.x * FIXED_TIMESTEP;
				if (!entities[i].isStatic) {
					collisionx(entities[i]);
				}
				entities[i].position.y += entities[i].velocity.y * FIXED_TIMESTEP;
				if (!entities[i].isStatic) {
					collisiony(entities[i]);
				}
			}*/

			////////ENTITY COLLISION//////////////////////////////////////////////////////////////////////////
			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].type == "player") {
					for (int j = 0; j < entities.size(); ++j) {
						if (entities[j].type == "gear" && entityCollision(entities[i], entities[j])) {
							entities.erase(entities.begin() + j);
							break;
						}
					}
					break;
				}
			}

			///////DRAWING////////////////////////////////////////////////////////////////////////////////////
			glClear(GL_COLOR_BUFFER_BIT);
			for (int y = 0; y < mapHeight; ++y) {
				for (int x = 0; x < mapWidth; ++x) {
					modelMatrix.identity();
					//modelMatrix.Translate(x*TILE_SIZE - (3.55 - TILE_SIZE/2),(32-y - 1)*TILE_SIZE - (2.0f - TILE_SIZE/2), 0);
					modelMatrix.Translate(x*TILE_SIZE, (mapHeight - y - 1)*TILE_SIZE, 0);
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);
					DrawSpriteSheetSprite(&program, levelData[y][x], 20, 10, goldenGearSpriteSheet);
				}
			}
			std::vector<std::string> types = {"gear", "goldenGear", "crab", "star", "bullet"};
			for (int i = 0; i < entities.size(); ++i) {
				modelMatrix.identity();
				//modelMatrix.Translate(entities[i].position.x - 3.55 + TILE_SIZE / 2, entities[i].position.y + TILE_SIZE / 2 + (mapHeight*TILE_SIZE - 2.0f), 0);
				modelMatrix.Translate(entities[i].position.x, entities[i].position.y + mapHeight*TILE_SIZE, 0);
				if (entities[i].type == "gear") {
					modelMatrix.Rotate(-1*ticks);
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
			SDL_GL_SwapWindow(displayWindow);
			break;
		}
	}

	SDL_Quit();
	return 0;
}