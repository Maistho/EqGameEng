#ifndef TILESYSTEM_H
#define TILESYSTEM_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

/* Heigth and width of the map in tiles, then heigth and width of the tiles in pixels.
	Make sure these values are correct according to images and map files. */
const int MAP_H = 15, MAP_W = 20;
const int TILE_H = 48, TILE_W = 48;
//const char* PLAYER_IMAGE = "player.png";
#define PLAYER_IMAGE "player.png"

class World;

class Sprite
{
	public:
		SDL_Surface* getSurface();
		~Sprite();
		Sprite(std::string sprite_img = NULL);
		SDL_Surface* sprite;

};

class Tile : public Sprite
{
	public:
		Tile(std::string sprite_img, bool solid_in);
		bool isPassable();
	private:
		bool solid;
};

class Object : public Sprite
{
	public:
		Object(int x_in, int y_in, int ID_in, std::string name_in, std::string sprite_img, bool storable_in, bool solid_in, bool movable_in);
		virtual bool move(char dir, World* world); //(n)orth, (s)outh, (w)est, (e)ast
		void absoluteMove(int x, int y, World* world);
		int getID();
		int getX();
		int getY();
		virtual int getBX() {return x;};
		virtual int getBY() {return y;};
		virtual int getTX() {return x;};
		virtual int getTY() {return y;};
		std::string getName();
		bool isMovable();
		bool isPassable();
		virtual bool changeScreen(char dir, World* world) {return false;};
		virtual std::vector<std::string>* getDialogue(World* world) {return nullptr;};
	protected:
		int x;
		int y;
		int ID;
		std::string name;
		bool storable;
		bool solid;
		bool movable;
};

class NPC : public Object
{
	public:
		NPC(int x_in, int y_in, int bx_in, int by_in, int ID_in, std::string name, std::string sprite_img);
		std::vector<std::string>* getDialogue(World* world);
		bool move(char dir, World* world); //(n)orth, (s)outh, (w)est, (e)ast
		int getTX() {return t_x;};
		int getTY() {return t_y;};
		int getBX() {return b_x;};
		int getBY() {return b_y;};

	private:
		int t_x;
		int t_y;
		int b_x;
		int b_y;
		int state = 0;
		std::map<int, std::pair<bool, std::vector<std::vector<std::string>>>> dialogue_map;
};

class Player : public Object
{
	public:
		Player(int x_in, int y_in);
		//		int getX();
		//		int getY();
	private:
		bool changeScreen(char dir, World* world);

};

class Screen
{
	public:
		Screen(std::string fname, Player* player, World* world);
		~Screen();
		Tile* getTile(int x, int y);
		std::vector<Object*>* getObjects(int x, int y);
	private:
		int x_pos;
		int y_pos;
		int tile_count;
		std::vector<Tile*> types;
		Tile* tiles[MAP_W][MAP_H];
		std::vector<Object*> objects[MAP_W][MAP_H];
		//		std::vector<std::vector<std::vector<Object*>>>> objects;

};

class World
{
	public:
		friend class Screen;
		World(int x = 0, int y = 0);
		~World();
		Screen* getScreen();
		void run();
		void render();
		void move(char dir); //(n)orth, (s)outh, (w)est, (e)ast
		int passable(int x, int y);
		int getState(int ID);
		int getX();
		int getY();
		void speak(int x, int y);
	private:
		void npcs_move();
		bool talking = false;
		void load();
		int x_pos;
		int y_pos;
		SDL_Surface* active_screen;
		SDL_Surface* text_field;
		SDL_Surface* npc_text;
		std::vector<SDL_Surface*> text_choices;
		TTF_Font* font;
		SDL_Color text_color;
		SDL_Color text_color_active;
		Screen* current_screen = nullptr;
		Player* player;
		std::map<int, std::vector<std::string>> objects_map;
};
#endif
