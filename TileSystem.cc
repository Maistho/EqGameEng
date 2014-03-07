#include "TileSystem.h"

Sprite::Sprite(std::string sprite_img)
{
	/* Loads the given filename as an image to store in the sprite */
	sprite = IMG_Load(("images/"+sprite_img).c_str());
	//	SDL_SoftStretch(temp, NULL, sprite, NULL);
}

Sprite::~Sprite()
{
	SDL_FreeSurface(sprite);
}

SDL_Surface* Sprite::getSurface()
{
	return sprite;
}

Tile::Tile(std::string sprite_img, bool solid_in) : Sprite(sprite_img), solid(solid_in) {}

bool Tile::isPassable()
{
	return solid;
}

Screen::Screen(std::string fname, Player* player, World* world)
{
	/* Takes two filenames as inputs, one for the map and one for the object overlay,
		and reads the files creating the screen */
	std::ifstream map_file;
	map_file.open(("maps/" + fname + ".map").c_str());
	if(!map_file) //Error if file doesn't exist
	{
		std::cerr << "File "<< fname << ".map does not exist or is empty." << std::endl;
	}
	else
	{
		/* Code for reading the map file. First element in the file is an integer telling
			how many different tiles the map contains. After that the format on the first line
is: <image_filename> <1/0> (for passable or non-passable). Finally, the map is
described line by line by integers, where 0 is the first tile read. */
		tile_count = 0;
		map_file >> tile_count;
		for(int i = 0; i < tile_count; ++i)
		{
			std::string tile_fname;
			bool passable;
			map_file >> tile_fname >> passable;
			types.push_back(new Tile(tile_fname.c_str(), passable));
		}
		map_file.ignore(1000, '\n');
		for(int y = 0; y < MAP_H; ++y)
		{
			for(int x = 0; x < MAP_W; ++x)
			{
				int type;
				map_file >> type;
				tiles[x][y] = types.at(type);
			}
			map_file.ignore(1000, '\n');
		}
	}
	map_file.close();
	std::ifstream obj_file;

	obj_file.open(("saves/" + fname + ".obj").c_str());
	if(!obj_file) //If saved object file doesn't exist, load default
	{
		obj_file.open(("objects/" + fname + ".obj").c_str());
	}
	if(!obj_file) //Error if file doesn't exist.
	{
		std::cerr << "File "<< fname << ".obj does not exist or is empty." << std::endl;
	}
	else
	{
		/* Code for reading the object file. First element in the file is an integer telling
			how many different objects the screen contains. After that, each integer on the first
			line tells what line in the list file contains the object data. Finally, each
			consecutive line */
		while(obj_file.peek() != EOF)
		{
			int obj_ID, x, y;
			obj_file >> x >> y >> obj_ID;
			if(obj_ID < 1000)
			{
				int b_x, b_y;
				obj_file >> b_x >> b_y;
				NPC* tmp_ptr = new NPC(x, y, b_x, b_y, obj_ID,
						world->objects_map[obj_ID].at(1),
						world->objects_map[obj_ID].at(2));
				objects[x][y].push_back(tmp_ptr);
			}
			else
			{
				Object* tmp_ptr = new Object(x, y, obj_ID, world->objects_map[obj_ID].at(1),
						world->objects_map[obj_ID].at(2), world->objects_map[obj_ID].at(3) == "true",
						world->objects_map[obj_ID].at(4) == "true",
						world->objects_map[obj_ID].at(5) == "true");
				objects[x][y].push_back(tmp_ptr);

			}
			obj_file.ignore(1000, '\n');
		}
	}
	obj_file.close();

	objects[player->getX()][player->getY()].push_back(player);

	x_pos = world->x_pos;
	y_pos = world->y_pos;
}

Screen::~Screen()
{
	for(unsigned int i = 0; i < types.size(); ++i)
	{
		delete types.at(i);
	}
	types.clear();
	//When a screen is removed, save objects to corresponding .obj-file in saves/.
	std::ofstream obj_save;
	obj_save.open(("saves/" + std::to_string(x_pos) + "_" + std::to_string(y_pos) + ".obj").c_str(), std::ios::out | std::ios::trunc);
	for(int y = 0; y < MAP_H; ++y)
	{
		for(int x = 0; x < MAP_W; ++x)
		{
			if(!getObjects(x, y)->empty())
			{
				for_each(getObjects(x, y)->begin(), getObjects(x, y)->end(),
						[x, y, &obj_save] (Object* object)
						{
						if(object->getID() > 1000) //Save an object.
						{
						obj_save << object->getX() << " "
						<< object->getY() << " "
						<< object->getID() << std::endl;
						}
						else if(object->getID() > 0) //Don't save the player to file, but save NPC's
						{
						obj_save << object->getTX() << " "
						<< object->getTY() << " "
						<< object->getID() << " "
						<< object->getBX() << " "
						<< object->getBY() << std::endl;
						}
						});
			}
		}
	}

}

Tile* Screen::getTile(int x, int y)
{
	return tiles[x][y];
}

std::vector<Object*>* Screen::getObjects(int x, int y)
{
	return &objects[x][y];
}

World::World(int x, int y) : x_pos(x), y_pos(y)
{
	player = new Player(9,7);
	{
		std::ifstream world_pos;
		world_pos.open("saves/pos.sav");
		if(world_pos)
		{
			world_pos >> x_pos >> y_pos;
		}
		world_pos.close();
		std::ifstream obj_list;
		obj_list.open("objects/objects.list");
		if(!obj_list)
		{
			std::cerr << "objects.list not found. Have you installed correctly?" << std::endl;
		}
		else
		{
			while(obj_list.peek() != EOF)
			{
				int ID;
				std::vector<std::string>* temp = new std::vector<std::string>;
				std::string tmp_string;
				obj_list >> ID;
				temp->push_back("0");
				if(ID > 1000) //Load a general object
				{
					for(int i = 0; i < 5; ++i)
					{
						obj_list >> tmp_string;
						temp->push_back(tmp_string);
					}
					objects_map.insert(std::pair<int, std::vector<std::string>>(ID, *temp));
				}
				else if(ID > 0) //Load an NPC
				{
					for(int i = 0; i < 2; ++i)
					{
						obj_list >> tmp_string;
						temp->push_back(tmp_string);
					}
					objects_map.insert(std::pair<int, std::vector<std::string>>(ID, *temp));
				}
				else
				{
					std::cerr << "Invalid ID in objects.list" << std::endl;
				}
				obj_list.ignore(1000, '\n');
			}

		}
		obj_list.close();
		std::ifstream saved_states;
		saved_states.open("saves/states.sav");
		if(saved_states)
		{
			while(saved_states.peek() != EOF)
			{
				int ID;
				std::string state;
				saved_states >> ID >> state;
				objects_map[ID].at(0) = state;
				saved_states.ignore(1000, '\n');
			}
		}
		saved_states.close();
	}
	load();

	if (SDL_Init(SDL_INIT_VIDEO /*|SDL_INIT_AUDIO*/) == -1)
	{
		std::cerr << "Error initializing SDL" << std::endl;
	}
	if( TTF_Init() == -1 )
	{
		std::cerr << "Error initializing TTF" << std::endl;
	}

	font = TTF_OpenFont("images/Equestria.ttf", 24);
	if(font == NULL)
	{
		std::cerr << "Couldn't open font." << std::endl;
	}

	text_color = {255, 255, 255, 0};
	text_color_active = {0, 255, 0, 0};

	SDL_WM_SetCaption( "My Little Pony: Friendship is magic. A Canterlot Wedding.", NULL );
	SDL_ShowCursor(0);
	active_screen = SDL_SetVideoMode(MAP_W * TILE_W, MAP_H * TILE_H, 32, SDL_SWSURFACE|SDL_DOUBLEBUF);
	if (active_screen == NULL)
	{
		std::cerr << "Error " << std::endl;
	}
}

World::~World()
{
	delete current_screen;
	std::ofstream saved_states;
	saved_states.open("saves/states.sav", std::ios::out | std::ios::trunc);
	for(std::map<int, std::vector<std::string>>::iterator it = objects_map.begin(); it != objects_map.end(); ++it)
	{
		if(it->second.at(0) != "0")
		{
			saved_states << it->first << " " << it->second.at(0) << std::endl;
		}
	}
	saved_states.close();

	std::ofstream world_pos;
	world_pos.open("saves/pos.sav", std::ios::out | std::ios::trunc);
	world_pos << x_pos << " " << y_pos;
	world_pos.close();

	SDL_Quit();
	TTF_Quit();
}

int World::getX()
{
	return x_pos;
}

int World::getY()
{
	return y_pos;
}

int World::getState(int ID)
{
	return std::stoi(objects_map[ID].at(0));
}


void World::speak(int x, int y)
{
	std::vector<std::string>* dialogue;
	Object* current_object = current_screen->getObjects(x, y)->back();
	talking = true;
	unsigned short int active = 0;
	std::string statemap[3]; //Max 3 choices at a time.

	while(talking)
	{
		dialogue = current_object->getDialogue(this);
		npc_text = TTF_RenderText_Solid(font, (current_object->getName() + ": " + dialogue->at(0)).c_str(), text_color);
		text_choices.clear();
		unsigned short int i = 0;
		for(std::vector<std::string>::iterator it = dialogue->begin()+1; it != dialogue->end(); ++it)
		{
			std::stringstream choice_stream(*it);
			choice_stream >> statemap[i];
			std::string choice_string;
			getline(choice_stream, choice_string);
			text_choices.push_back(TTF_RenderText_Solid(font, ("-" + choice_string).c_str(), (i == active) ? text_color_active : text_color));
			++i;
		}
		render();

		SDL_Event ev;
		bool awaiting_input = true;
		while(awaiting_input)
		{
			while(SDL_PollEvent(&ev))
			{
				if(ev.type == SDL_KEYDOWN)
				{
					if(ev.key.keysym.sym == SDLK_q)
					{
						talking = false;
					}
					else if(ev.key.keysym.sym == SDLK_UP)
					{
						if(active > 0)
						{
							--active;
						}
					}
					else if(ev.key.keysym.sym == SDLK_DOWN)
					{
						if(active < text_choices.size()-1)
						{
							++active;
						}
					}
					else if(ev.key.keysym.sym == SDLK_RETURN)
					{
						if(statemap[active] == "[break]")
						{
							talking = false;
						}
						else if(statemap[active][0] == '[')
						{
							std::stringstream temp_parser(statemap[active]);
							int player_x, player_y;
							temp_parser.ignore(1, '[');
							temp_parser >> x_pos;
							temp_parser.ignore(1, ',');
							temp_parser >> y_pos;
							temp_parser.ignore(1, ',');
							temp_parser >> player_x;
							temp_parser.ignore(1, ',');
							temp_parser >> player_y;
							talking = false;
							load();
							player->absoluteMove(player_x, player_y, this);
						}
						else
						{
							objects_map[current_object->getID()].at(0) = statemap[active];
							active = 0;
						}
					}
					awaiting_input = false;
				}
			}
		}
	}
}



Screen* World::getScreen()
{
	return current_screen;
}
void World::render()
{
	SDL_FillRect(active_screen, NULL, 0xFF00FF);
	for(int y = 0; y < MAP_H; ++y)
	{
		for(int x = 0; x < MAP_W; ++x)
		{
			SDL_Rect current_tile;
			current_tile.x = x * TILE_W;
			current_tile.y = y * TILE_H;
			SDL_BlitSurface(current_screen->getTile(x, y)->getSurface(), NULL, active_screen, &current_tile);

			if(!current_screen->getObjects(x, y)->empty())
			{
				for_each(current_screen->getObjects(x, y)->begin(), current_screen->getObjects(x, y)->end(),
						[&current_tile, this] (Object* object)
						{SDL_BlitSurface(object->getSurface(), NULL, active_screen, &current_tile);});
			}

			if(talking)
			{
				SDL_Rect textback = {0, MAP_H*TILE_H-150, MAP_W*TILE_W, 150};
				SDL_Rect text_area = { 30, MAP_H*TILE_H-140, MAP_W*TILE_W - 40, 140};
				SDL_FillRect(active_screen, &textback, 0x000000);
				SDL_BlitSurface(npc_text, NULL, active_screen, &text_area);

				for(unsigned int i = 0; i < text_choices.size(); ++i)
				{
					SDL_Rect temp_rect = {40, Sint16(MAP_H*TILE_H-140+(i+1)*26), MAP_W*TILE_W-50, Uint16(140-(i+1)*26)};
					SDL_BlitSurface(text_choices.at(i), NULL, active_screen, &temp_rect);
				}
			}
		}
	}

	SDL_Flip(active_screen);
	SDL_Delay(10);
}

int World::passable(int x, int y)
{
	if(x < 0 || y < 0 || x >= MAP_W || y >= MAP_H)
	{
		return 3;
	}
	else if(!current_screen->getObjects(x, y)->empty()
			&& (current_screen->getObjects(x, y)->back()->getID() < 1000
				&& current_screen->getObjects(x, y)->back()->getID() > 0))
	{
		return 4;
	}
	else if(!current_screen->getTile(x, y)->isPassable())
	{
		return 2;
	}
	else if(!current_screen->getObjects(x, y)->empty())
	{
		if(!current_screen->getObjects(x, y)->back()->isPassable())
		{
			if(!current_screen->getObjects(x, y)->back()->isMovable())
			{
				return 2;
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;

}

void World::run()
{
	bool running(true);
	while(running)
	{
		render();
		npcs_move();
		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		{
			if(ev.type == SDL_KEYDOWN)
			{
				if(ev.key.keysym.sym == SDLK_q)
				{
					running = false;
				}else if(ev.key.keysym.sym == SDLK_UP)
				{
					player->move('n', this);
				}else if(ev.key.keysym.sym == SDLK_DOWN)
				{
					player->move('s', this);
				}else if(ev.key.keysym.sym == SDLK_LEFT)
				{
					player->move('w', this);
				}else if(ev.key.keysym.sym == SDLK_RIGHT)
				{
					player->move('e', this);
				}
			}

		}
	}
}

void World::load()
{
	delete current_screen;
	// Creates a new screen from files x_y.map and x_y.obj
	current_screen = new Screen(
			(std::to_string(x_pos)+"_"+std::to_string(y_pos)), player, this);
}


void World::move(char dir) //(n)orth, (s)outh, (w)est, (e)ast
{
	switch(dir)
	{
		case 'n':
			++y_pos;
			break;
		case 's':
			--y_pos;
			break;
		case 'w':
			--x_pos;
			break;
		case 'e':
			++x_pos;
			break;
	}

	load();

}

void World::npcs_move()
{
	for(int y = 0; y < MAP_H; ++y)
	{
		for(int x = 0; x < MAP_W; ++x)
		{
			std::vector<Object*>* current_objects = current_screen->getObjects(x, y);
			if(current_objects->size() != 0)
			{
				if(current_objects->back()->getID() < 1000 && current_objects->back()->getID() > 0)
				{
					int random = rand() % 40;
					if(random == 1)
					{
						int dir = rand() % 4;
						char send;
						switch(dir)
						{
							case 0:
								send = 'n';
								break;
							case 1:
								send = 's';
								break;
							case 2:
								send = 'w';
								break;
							case 3:
								send = 'e';
								break;
						}
						current_objects->back()->move(send, this);
					}
				}
			}
		}
	}
}
