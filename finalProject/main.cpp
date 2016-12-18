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
#include <cmath>
#include <SDL_mixer.h> // Sound

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "assignment1.app/Contents/Resources/"
#endif

#define PI 3.141592653589793238f

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
		type(type), isStatic(isStatic), position(position), velocity(velocity), acceleration(acceleration), size(size), rotation(rotation), exists(true), 
		collidedTop(false), collidedBottom(false), collidedLeft(false), collidedRight(false), cliffLeft(false), cliffRight(false), cliffDown(false) {}
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
	bool cliffLeft;
	bool cliffRight;
	bool cliffDown;
	//SheetSprite sprite;
};

//class Particle {
//public:
//	Vector3 position;
//	Vector3 velocity;
//	float lifetime;
//};
//
//Vector3 pos(0.0f, 0.0f, 0.0f);
//Vector3 grav(5.0f, 5.0f, 0.0f);
//float lifeTime = 10000.0f;
//
//class ParticleEmitter {
//public:
//	ParticleEmitter(unsigned int particleCount);
//	ParticleEmitter() : position(pos), gravity(grav), maxLifetime(lifeTime) {}
//	//~ParticleEmitter();
//
//	void UpdatePart(float elapsed);
//	void RenderPart(ShaderProgram &program);
//
//	Vector3 position;
//	Vector3 gravity;
//	float maxLifetime;
//
//	std::vector<Particle> particles;
//};
//
//void ParticleEmitter::UpdatePart(float elapsed) {
//	for (Particle p : particles) {
//		p.position.x += p.velocity.x * 2.0f;
//	}
//}
//
//void ParticleEmitter::RenderPart(ShaderProgram &program) {
//	std::vector<float> vertices;
//	for (int i = 0; i < particles.size(); i++) {
//		vertices.push_back(particles[i].position.x);
//		vertices.push_back(particles[i].position.y);
//	}
//	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
//	glEnableVertexAttribArray(program.positionAttribute);
//	glDrawArrays(GL_POINTS, 0, vertices.size() / 2);
//}

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

float TILE_SIZE = .2f;

void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX, int spriteCountY, GLuint textureID) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0f / (float)spriteCountX;
	float spriteHeight = 1.0f / (float)spriteCountY;

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
	float ax = 0;
	if (type == "player" || type == "crab") {
		isStatic = false;
	}
	if (type == "player") {
		pIndex = entities.size();
	}
	if (type == "crab") {
		ax = -.5;
	}
	Entity entity(type, isStatic, Vector3(x, y, 0), Vector3(0, 0, 0), Vector3(ax, 0, 0));
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
	return (1.0f - t)*v0 + t*v1;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-(worldY) / TILE_SIZE);
}

enum GameState { STATE_MAIN_MENU, STATE_LEVEL_SELECT, STATE_CONTROLS, STATE_GAME_LEVEL, STATE_GAME_OVER };
int state = STATE_MAIN_MENU;

bool testOutOfBounds(int gridX, int gridY) {
	if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) {
		return true;
	}
	return false;
}

void handleOOB(Entity &entity) {
	if (entity.type == "player") {
		state = STATE_GAME_OVER;
	}
	else {
		entity.exists = false;
	}
}

void cliffx(Entity &entity) {
	int gridX, gridY;

	//check left
	worldToTileCoordinates(entity.position.x - 0.008f, entity.position.y - 0.008f, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] >= 18)
	{
		entity.cliffLeft = true;
	}

	//check right
	worldToTileCoordinates(entity.position.x + TILE_SIZE + 0.008f, entity.position.y - 0.008f, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] >= 18)
	{
		entity.cliffRight = true;
	}

	//check down
	worldToTileCoordinates(entity.position.x + TILE_SIZE / 2, entity.position.y - 0.008f, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] >= 18)
	{
		entity.cliffDown = true;
	}

}

void collisionx(Entity &entity, ShaderProgram &program) {
	int gridX, gridY;

	//check left
	worldToTileCoordinates(entity.position.x, entity.position.y + 0.008f, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1.0f * (entity.position.x) - gridX * TILE_SIZE - TILE_SIZE + 0.008f;
		entity.velocity.x = 0.0f;
		//entity.acceleration.x = 0;
		entity.collidedLeft = true;
	}
	worldToTileCoordinates(entity.position.x, entity.position.y + TILE_SIZE - 0.008f, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1.0f * (entity.position.x) - gridX * TILE_SIZE - TILE_SIZE + 0.008f;
		entity.velocity.x = 0.0f;
		//entity.acceleration.x = 0;
		entity.collidedLeft = true;
	}
	//check right
	worldToTileCoordinates(entity.position.x + TILE_SIZE, entity.position.y + 0.008f, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1.0f * (entity.position.x) - gridX * TILE_SIZE + TILE_SIZE - 0.008f;
		entity.velocity.x = 0.0f;
		//entity.acceleration.x = 0;
		entity.collidedRight = true;
	}
	worldToTileCoordinates(entity.position.x + TILE_SIZE, entity.position.y + TILE_SIZE - 0.008f, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18 || (entity.type == "player" && gridY - 1 >= 0 && levelData[gridY - 1][gridX] < 18))
	{
		entity.position.x += 1.0f * (entity.position.x) - gridX * TILE_SIZE + TILE_SIZE - 0.008f;
		entity.velocity.x = 0.0f;
		//entity.acceleration.x = 0;
		entity.collidedRight = true;
	}
}

float distance(double x1, double y1, double x2, double y2) {
	return (float)sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void breakWood(int x, int y) {
	if (!testOutOfBounds(x, y) && levelData[y][x] == 14) {
		levelData[y][x] = 18;
		breakWood(x + 1, y);
		breakWood(x - 1, y);
	}
}

Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

// Sound globals
Mix_Chunk *magnetRepel;
Mix_Chunk *magnetAttract;
Mix_Chunk *gearPickup;
Mix_Chunk *groundBreak;
Mix_Chunk *groundSmash;
Mix_Chunk *jump;
Mix_Chunk *landing;

float screenShakeValue;
float screenShakeIntensity;

void screenShake(float scale, ShaderProgram &program) {
	//add the particle effect in here too
	screenShakeIntensity = scale;
	screenShakeValue = 0.0f;
	//Mix_PlayChannel(-1, groundBreak, 0); //this is just for testing, remove it once its implemented
}

void killRadius(Entity &entity, float r) {
	for (unsigned int i = 0; i < entities.size(); ++i) {
		if (entities[i].exists && (distance(entity.position.x, entity.position.y, entities[i].position.x, entities[i].position.y) <= r) && (&entity != &entities[i]) &&
			entities[i].type != "gear" && entities[i].type != "silverGear" && entities[i].type != "goldenGear") {
			entities[i].exists = false;
		}
	}
	Mix_PlayChannel(-1, groundBreak, 0);
}

void collisiony(Entity &entity, ShaderProgram &program) {
	int gridX, gridY;
	float top = 0.0f;
	if (entity.type == "player") {
		top = TILE_SIZE;
	}
	//check bottom
	worldToTileCoordinates(entity.position.x + 0.008f, entity.position.y, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		if (entity.type == "player" && entity.velocity.y < -1.0) {
			Mix_PlayChannel(-1, landing, 0);
			screenShake(entity.velocity.y / 100.0f, program);
		}
		if (entity.type == "player" && entity.velocity.y <= -7.0f && levelData[gridY][gridX] == 14) {
			breakWood(gridX, gridY);
		}
		else {
			if (entity.type == "player" && entity.velocity.y <= -7.0f) {
				Mix_PlayChannel(-1, groundSmash, 0);
				killRadius(entity, 2.5f * TILE_SIZE);
			}
			entity.position.y += -1.0f * (entity.position.y) - gridY * TILE_SIZE + 0.001f;
			entity.velocity.y = 0.0f;
			entity.collidedBottom = true;
		}
	}
	worldToTileCoordinates(entity.position.x + TILE_SIZE - 0.008f, entity.position.y, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		entity.position.y += -1.0f * (entity.position.y) - gridY * TILE_SIZE + 0.001f;
		entity.velocity.y = 0.0f;
		entity.collidedBottom = true;
	}
	//check top
	worldToTileCoordinates(entity.position.x + 0.008f, entity.position.y + TILE_SIZE + top, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		entity.position.y += -1.0f * (entity.position.y + TILE_SIZE + top) - gridY * TILE_SIZE - TILE_SIZE - 0.001f;
		entity.velocity.y = 0.0f;
		entity.collidedTop = true;
	}
	worldToTileCoordinates(entity.position.x + TILE_SIZE - 0.008f, entity.position.y + TILE_SIZE + top, &gridX, &gridY);
	if (testOutOfBounds(gridX, gridY)) {
		handleOOB(entity);
		return;
	}
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 18)
	{
		entity.position.y += -1.0f * (entity.position.y + TILE_SIZE + top) - gridY * TILE_SIZE - TILE_SIZE - 0.001f;
		entity.velocity.y = 0.0f;
		entity.collidedTop = true;
	}
}

bool entityCollision(Entity &e1, Entity &e2) {
	float top = 0.0f;
	if (e1.type == "player") {
		top = TILE_SIZE;
	}
	return (!(
		e1.position.x - TILE_SIZE / 2 > e2.position.x + TILE_SIZE / 2 ||
		e1.position.x + TILE_SIZE / 2 < e2.position.x - TILE_SIZE / 2 ||
		e1.position.y - TILE_SIZE / 2 > e2.position.y + TILE_SIZE / 2 ||
		e1.position.y + TILE_SIZE / 2 + top < e2.position.y - TILE_SIZE / 2));
}

float p1ax;
float p1ay;
float p1vx;
float p1vy;
float p1jump;
float cooldown;

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
void Update(float ticks, float time, ShaderProgram &program) {
	for (unsigned int i = 0; i < entities.size(); ++i) {

		bool prevGrounded = true;

		if (entities[i].type == "player") {

			prevGrounded = entities[i].collidedBottom;

			entities[i].acceleration.x = p1ax;
			//entities[i].acceleration.y = p1ay;

			if (entities[i].collidedBottom) {
				if (p1jump != 0.0f) {
					Mix_PlayChannel(-1, jump, 0);
				}
				entities[i].velocity.y += p1jump;
				p1jump = 0.0f;
			}
				entities[i].velocity.x += p1vx;
				entities[i].velocity.y += p1vy;
				p1vy = 0.0f;
				p1vx = 0.0f;

			if (entities[i].collidedLeft || entities[i].collidedRight) {
				entities[i].acceleration.x = 0;
			}
		}
		else if (entities[i].type == "gear" || entities[i].type == "silverGear") {
			entities[i].position.y += 0.001f * sin(ticks * 10.0f / PI);
		}
		else if (entities[i].type == "crab") {
			if (entities[i].cliffDown);			//catches midair crabs so they don't get confused
			else if (entities[i].collidedLeft || entities[i].cliffLeft) {
				entities[i].acceleration.x = 0.5f;
				entities[i].velocity.y = 1.0;
			}
			else if (entities[i].collidedRight || entities[i].cliffRight) {
				entities[i].acceleration.x = -0.5f;
				entities[i].velocity.y = 1.0;
			}
		}

		//gravity
		if (!entities[i].isStatic) {
			entities[i].acceleration.y = -5.0f;
		}

		if (entities[i].type == "player") {
			entities[i].acceleration.y = p1ay;
		}
		
		entities[i].collidedTop = false;
		entities[i].collidedBottom = false;
		entities[i].collidedLeft = false;
		entities[i].collidedRight = false;
		entities[i].cliffLeft = false;
		entities[i].cliffRight = false;
		entities[i].cliffDown = false;

		entities[i].velocity.x = lerp(entities[i].velocity.x, 0.0f, time * 5);
		entities[i].velocity.y = lerp(entities[i].velocity.y, 0.0f, time * 1);

		entities[i].velocity.x += entities[i].acceleration.x * time;
		entities[i].velocity.y += entities[i].acceleration.y * time;

		entities[i].position.y += entities[i].velocity.y * time;
		if (!entities[i].isStatic) {
			collisiony(entities[i], program);
		}
		entities[i].position.x += entities[i].velocity.x * time;
		if (!entities[i].isStatic) {
			collisionx(entities[i], program);
			cliffx(entities[i]);
		}
	}
}

enum letterIndex { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z };
int LETTER_SHIFT = 65;

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
				//state = STATE_GAME_LEVEL;
				state = STATE_LEVEL_SELECT;
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
			modelMatrix.Translate(-3.45f + i*.2f + 1.4f, 0.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, playGameArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 8; ++i) { // Length of controlsArr
		if (controlsArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45f + i*.2f + 1.4f, -0.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, controlsArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 4; ++i) { // Length of exitArr
		if (exitArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45f + i*.2f + 1.4f, -1.0f, 0.0f);
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

	float vertices[] = { -3.55f, -2.0f, 3.55f, -2.0f, 3.55f, 2.0f, -3.55f, -2.0f, 3.55f, 2.0f, -3.55f, 2.0f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	float texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	SDL_GL_SwapWindow(displayWindow);
}

void new_game(ShaderProgram &program, const std::string &level) {
	std::ifstream world(level);
	std::string line;
	while (getline(world, line)) {
		if (line == "[header]") {
			if (!readHeader(world)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(world);
		}
		else if (line == "[objectLayer]") {
			readEntityData(world);
		}
	}
	world.close();
}

std::string level;

void level_select(ShaderProgram &program, GLuint &letters) {
	int stageOneArr[] = { S, T, A, G, E, -1, O, N, E };
	int stageTwoArr[] = { S, T, A, G, E, -1, T, W, O };
	int stageThreeArr[] = { S, T, A, G, E, -1, T, H, R, E, E };
	int backArr[] = { B, A, C, K };

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
				level = "world1.txt";
				new_game(program, level);
				state = STATE_GAME_LEVEL;
			}

			else if (units_x > -2.1f && units_x < -0.4f && units_y > -0.6f && units_y < -0.4f) {
				level = "test.txt";
				new_game(program, level);
				state = STATE_GAME_LEVEL;
			}

			else if (units_x > -2.1f && units_x < 0.0f && units_y > -1.1f && units_y < -0.9f) {
				level = "world1.txt";
				new_game(program, level);
				state = STATE_GAME_LEVEL;
			}

			else if (units_x > -2.1f && units_x < -1.4f && units_y > -1.6f && units_y < -1.4f) {
				state = STATE_MAIN_MENU;
			}
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < 9; ++i) { // Length of stageOneArr
		if (stageOneArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45f + i*.2f + 1.4f, 0.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, stageOneArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 9; ++i) { // Length of stageTwoArr
		if (stageTwoArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45f + i*.2f + 1.4f, -0.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, stageTwoArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 11; ++i) { // Length of stageThreeArr
		if (stageThreeArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45f + i*.2f + 1.4f, -1.0f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, stageThreeArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 4; ++i) { // Length of backArr
		if (backArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-3.45f + i*.2f + 1.4f, -1.5f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, backArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	SDL_GL_SwapWindow(displayWindow);
}

void game_over(ShaderProgram &program, GLuint &gameOverPage, GLuint &letters, std::string &level) {

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
				new_game(program, level);
				state = STATE_GAME_LEVEL;
			}

			if (units_x > -0.4f && units_x < 0.3f && units_y > -1.4f && units_y < -1.2f) {
				state = STATE_MAIN_MENU;
			}
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);

	viewMatrix.identity();
	modelMatrix.identity();
	// Draw background image
	program.setModelMatrix(modelMatrix);
	program.setProjectionMatrix(projectionMatrix);
	program.setViewMatrix(viewMatrix);

	//GLuint gameOverPage = LoadTexture("game_over.png");

	glBindTexture(GL_TEXTURE_2D, gameOverPage);

	float vertices[] = { -3.55f, -2.0f, 3.55f, -2.0f, 3.55f, 2.0f, -3.55f, -2.0f, 3.55f, 2.0f, -3.55f, 2.0f };
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
			modelMatrix.Translate(-0.9f + i*.2f, -0.8f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, playAgainArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	for (int i = 0; i < 4; ++i) { // Length of exitArr
		if (exitArr[i] != -1) {
			modelMatrix.identity();
			modelMatrix.Translate(-0.3f + i*.2f, -1.3f, 0.0f);
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			DrawSpriteSheetSprite(&program, exitArr[i] + LETTER_SHIFT, 16, 16, letters);
		}
	}

	modelMatrix.identity();

	SDL_GL_SwapWindow(displayWindow);
}


float units_x = 0.0f;
float units_y = 0.0f;
float units_mx = 0.0f;
float units_my = 0.0f;
double minDist = 1000000000;
int minx = -1;
int miny = -1;
float elapsed = 0.0f;
float lastFrameTicks = 0.0f;
float ticks = 0.0f;
float fixedElapsed = 0.0f;
int gearCount = 0;
int NUM_SHIFT = 48;

void level_clear() {
	//empty the level
	entities.clear();
	for (int i = 0; i < mapHeight; ++i) {
		delete[] levelData[i];
	}
	delete[] levelData;
	gearCount = 0;
	p1vx = 0.0f;
	p1vy = 0.0f;
	p1ax = 0.0f;
	p1ay = -5.0f;
	p1jump = 0.0f;
	cooldown = 0.0f;
	units_x = 0.0f;
	units_y = 0.0f;
	units_mx = 0.0f;
	units_my = 0.0f;
	minDist = 1000000000;
	minx = -1;
	miny = -1;
	elapsed = 0.0f;
	lastFrameTicks = 0.0f;
	ticks = 0.0f;
	fixedElapsed = 0.0f;

	screenShakeValue = 0.0f;
	screenShakeIntensity = 0.0f;

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
	
	GLuint controlsPage = LoadTexture("controls.png");
	GLuint gameOverPage = LoadTexture("game_over.png");
	GLuint letters = LoadTexture("letters.png");
	GLuint goldenGearSpriteSheet = LoadTexture("golden_gear_spritesheet.png");

	//new_game(program, level);

	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	p1vx = 0.0f;
	p1vy = 0.0f;
	p1ax = 0.0f;
	p1ay = -5.0f;
	p1jump = 0.0f;
	cooldown = 0.0f;
	units_x = 0.0f;
	units_y = 0.0f;
	units_mx = 0.0f;
	units_my = 0.0f;
	minDist = 1000000000;
	minx = -1;
	miny = -1;
	elapsed = 0.0f;
	lastFrameTicks = 0.0f;
	ticks = 0.0f;
	fixedElapsed = 0.0f;

	screenShakeValue = 0.0f;
	screenShakeIntensity = 0.0f;

	//int gearCount = 0; // ----------------------------------------
	int gearCountArr[] = { NUM_SHIFT, NUM_SHIFT, NUM_SHIFT };

	//// Loading sound and music
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	magnetRepel = Mix_LoadWAV("magnetRepel.wav");
	Mix_VolumeChunk(magnetRepel, 100);
	magnetAttract = Mix_LoadWAV("magnetAttract.wav");
	Mix_VolumeChunk(magnetAttract, 100);
	gearPickup = Mix_LoadWAV("gearPickup.wav");
	Mix_VolumeChunk(gearPickup, 10000000000);
	groundBreak = Mix_LoadWAV("groundBreak.wav");
	Mix_VolumeChunk(groundBreak, 100);
	groundSmash = Mix_LoadWAV("groundSmash.wav");
	Mix_VolumeChunk(groundSmash, 100);
	jump = Mix_LoadWAV("jump.wav");
	Mix_VolumeChunk(jump, 50);
	landing = Mix_LoadWAV("landing.wav");
	Mix_VolumeChunk(landing, 50);

	Mix_Music *gameMusic;
	gameMusic = Mix_LoadMUS("gameMusic.mp3");
	Mix_VolumeMusic(50);
	Mix_PlayMusic(gameMusic, -1);
	//Mix_Music *menuMusic;
	//menuMusic = Mix_LoadMUS("menuMusic.mp3");

	//ParticleEmitter p;
	//for (int i = 0; i < 1000; ++i) {
	//	Particle part;
	//	part.position = pos;
	//	part.velocity = grav;
	//	part.lifetime = lifeTime;
	//	p.particles.push_back(part);
	//	//pos.x += 0.002f;
	//}
	//p.UpdatePart(100.0f);
	//p.RenderPart(program);

	while (!done) {

		switch (state) {

		case STATE_MAIN_MENU:
			main_menu(program, letters);
			break;

		case STATE_LEVEL_SELECT:
			level_select(program, letters);
			break;

		case STATE_CONTROLS:
			controls(program, controlsPage);
			//game_over(program, gameOverPage, letters);
			break;

		case STATE_GAME_OVER:
			game_over(program, gameOverPage, letters, level);
			break;

		case STATE_GAME_LEVEL:

			/////////////INPUT////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			p1ax = 0.0f;
			p1ay = -5.0f;

			int p1x, p1y;
			worldToTileCoordinates(entities[pIndex].position.x + TILE_SIZE / 2, entities[pIndex].position.y + TILE_SIZE / 2, &p1x, &p1y);

			const Uint8 mouse = SDL_GetMouseState(NULL, NULL);
			if ((mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) && ((p1x == minx && p1y + 1 == miny) || ((p1y == miny || p1y - 1 == miny) && p1x + 10 >= minx) ||
				((p1y == miny || p1y - 1 == miny) && p1x - 10 <= minx) || (p1x == minx))) {
				switch (levelData[miny][minx]) {
				case 9:
					if (p1x == minx && p1y + 10 >= miny && p1y < miny) {
						p1ay = -10.0f;
						if ((int)(ticks * 1000) % 18 == 0) Mix_PlayChannel(-1, magnetAttract, 0);
					}
					break;
				case 10:
					if ((p1y == miny || p1y - 1 == miny) && p1x + 10 >= minx && p1x < minx) {
						p1ax = 5.0f;
						entities[pIndex].velocity.y = 0.0f;
						p1ay = 0.0f;
						if ((int)(ticks * 1000) % 18 == 0) Mix_PlayChannel(-1, magnetAttract, 0);
					}
					break;
				case 11:
					if ((p1y == miny || p1y - 1 == miny) && p1x - 10 <= minx && p1x > minx) {
						p1ax = -5.0f;
						entities[pIndex].velocity.y = 0.0f;
						p1ay = 0.0f;
						if ((int)(ticks * 1000) % 18 == 0) Mix_PlayChannel(-1, magnetAttract, 0);
					}
					break;
				case 12:
					if (p1x == minx && p1y - 10 <= miny && p1y > miny) {
						p1ay = 5.0f;
						if ((int)(ticks * 1000) % 18 == 0) Mix_PlayChannel(-1, magnetAttract, 0);
					}
					break;
				default:
					break;
				}
			}
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_K]) {
				p1ax += -5.0f;
			}
			if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_SEMICOLON]) {
				p1ax += 5.0f;
			}

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1jump = 3.1f;
						//entities[pIndex].velocity.y = 3.1f;
					}
				}
				else if (event.type == SDL_KEYUP) {
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1jump = 0.0f;
					}
				}
				else if (event.type == SDL_MOUSEMOTION) {
					units_x = (((float)event.motion.x / 1280) * 7.1f) - 3.55f;
					units_y = (((float)(720 - event.motion.y) / 720) * 4.0f) - 2.0f;
				}
				else if (event.type == SDL_MOUSEBUTTONDOWN) {
					if (event.button.button == 3) {
						switch (levelData[miny][minx]) {
						case 9:
							if (p1x == minx && p1y + 1 == miny) {
								p1vy = 7.0f;
								Mix_PlayChannel(-1, magnetRepel, 0);
							}
							break;
						case 10:
							if ((p1y == miny || p1y - 1 == miny) && p1x + 1 == minx) {
								p1vx = -14.0f;
								Mix_PlayChannel(-1, magnetRepel, 0);
							}
							break;
						case 11:
							if ((p1y == miny || p1y - 1 == miny) && p1x - 1 == minx) {
								p1vx = 14.0;
								Mix_PlayChannel(-1, magnetRepel, 0);
							}
							break;
						case 12:
							if (p1x == minx && p1y - 2 == miny) {
								p1vy = -10.0f;
								Mix_PlayChannel(-1, magnetRepel, 0);
							}
							break;
						default:
							break;
						}
					}
				}
				else if (event.type == SDL_MOUSEBUTTONUP) {
					if (event.button.button == 3) {
						p1vy = 0.0f;
						p1vx = 0.0f;
					}
				}
			}
			

			////////MOVEMENT//////////////////////////////////////////////////////////////////////////////////
			//timestep
			ticks = (float)SDL_GetTicks() / 1000.0f;
			elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			// 60 FPS (1.0f/60.0f)
			fixedElapsed = elapsed;
			if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
				fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
			}
			while (fixedElapsed >= FIXED_TIMESTEP) {
				fixedElapsed -= FIXED_TIMESTEP;
				Update(ticks, FIXED_TIMESTEP, program);
			}
			Update(ticks, fixedElapsed, program);
			units_mx = units_x + entities[pIndex].position.x + TILE_SIZE / 2;
			units_my = units_y + entities[pIndex].position.y + mapHeight*TILE_SIZE + TILE_SIZE / 2;

			screenShakeValue += elapsed;
			if (screenShakeValue > 0.25f) {
				screenShakeIntensity = 0.0f;
			}

			////////ENTITY COLLISION//////////////////////////////////////////////////////////////////////////
			for (unsigned int i = 0; i < entities.size(); ++i) {
				if (entities[i].type == "player") {
					for (unsigned int j = 0; j < entities.size(); ++j) {
						if (entities[j].exists) {

							if (entities[j].type == "gear" && entityCollision(entities[i], entities[j])) {
								entities[j].exists = false;
								gearCount++;
								Mix_PlayChannel(-1, gearPickup, 0);
								break;
							}
							else if (entities[j].type == "silverGear" && entityCollision(entities[i], entities[j])) {
								entities[j].exists = false;
								gearCount += 10;
								Mix_PlayChannel(-1, gearPickup, 0);
								break;
							}
							else if (entities[j].type == "crab" && entityCollision(entities[i], entities[j])) {
								if (entities[i].velocity.y <= -7.0f) {
									entities[j].exists = false;
								}
								else {
									state = STATE_GAME_OVER;
								}
								break;
							}

						}
					}
					break;
				}
			}

			///////DRAWING////////////////////////////////////////////////////////////////////////////////////
			glClear(GL_COLOR_BUFFER_BIT);

			minDist = 1000000000;
			minx = -1;
			miny = -1;

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

					worldToTileCoordinates(units_mx, units_my, &mx, &my);
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
						DrawSpriteSheetSprite(&program, 26, 20, 10, goldenGearSpriteSheet);
					}*/

				}
			}
			//draw target on nearest magnet
			if (minx >= 0 && miny >= 0) {
				modelMatrix.identity();
				modelMatrix.Translate(minx*TILE_SIZE, (mapHeight - miny - 1)*TILE_SIZE, 0);
				modelMatrix.Rotate(-5 * ticks);
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
			DrawSpriteSheetSprite(&program, 26, 20, 10, goldenGearSpriteSheet);*/
			
			std::vector<std::string> types = {"gear", "silverGear", "goldenGear", "target", "crab", "star", "bullet"};
			for (unsigned int i = 0; i < entities.size(); ++i) {
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
						viewMatrix.Translate(-entities[i].position.x, (-1 * (entities[i].position.y + mapHeight*TILE_SIZE))  + sin(screenShakeValue * 100.0f) * screenShakeIntensity, 0);
					}
					else {
						int pos = find(types.begin(), types.end(), entities[i].type) - types.begin();
						DrawSpriteSheetSprite(&program, pos + 24, 20, 10, goldenGearSpriteSheet);
					}
				}
			}			

			gearCountArr[0] = NUM_SHIFT + gearCount / 100;
			gearCountArr[1] = NUM_SHIFT + gearCount / 10 % 10;
			gearCountArr[2] = NUM_SHIFT + gearCount % 10;

			for (int i = 0; i < 3; ++i) { // Length of gearCountArr
				if (gearCountArr[i] != -1) {
					modelMatrix.identity();
					modelMatrix.Translate(entities[pIndex].position.x, entities[pIndex].position.y + mapHeight*TILE_SIZE, 0);
					modelMatrix.Translate(3.0f + i*.2f, 1.9f, 0.0f);
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);
					DrawSpriteSheetSprite(&program, gearCountArr[i], 16, 16, letters);
				}
			}

			SDL_GL_SwapWindow(displayWindow);
			if (state == STATE_GAME_OVER) {
				level_clear();
				break;
			}
		}
	}

	// Free music/sounds
	Mix_FreeMusic(gameMusic);
	Mix_FreeChunk(magnetRepel);
	Mix_FreeChunk(magnetAttract);
	Mix_FreeChunk(gearPickup);
	Mix_FreeChunk(groundBreak);
	Mix_FreeChunk(groundSmash);
	Mix_FreeChunk(jump);
	Mix_FreeChunk(landing);

	SDL_Quit();
	return 0;
}