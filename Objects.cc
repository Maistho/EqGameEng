#include "TileSystem.h"

Object::Object(int x_in, int y_in, int ID_in, std::string name_in, std::string sprite_img, bool storable_in, bool solid_in, bool movable_in)
	: Sprite(sprite_img), x(x_in), y(y_in), ID(ID_in), name(name_in), storable(storable_in), solid(solid_in), movable(movable_in)
	{}

bool Object::isMovable()
{
	return movable;
}
bool Object::isPassable()
{
	return !solid;
}

bool Object::move(char dir, World* world) //(n)orth, (s)outh, (w)est, (e)ast
{
	int x_ = x;
	int y_ = y;
	switch(dir)
	{
		case 'n':
			--y_;
			break;
		case 's':
			++y_;
			break;
		case 'w':
			--x_;
			break;
		case 'e':
			++x_;
			break;
	}
	
	switch(world->passable(x_, y_))
	{
		case 0: //Destination is free,
			absoluteMove(x_, y_, world);
			x = x_;
			y = y_;
			return true;
			break;
			
		case 1: //Destination is occupied by movable object
			if(world->getScreen()->getObjects(x_, y_)->back()->move(dir, world))
			{
				absoluteMove(x_, y_, world);
				x = x_;
				y = y_;
				return true;
			}
			else
			{
				return false;
			}
			break;
			
		case 2: //Destination is blocked
			return false;
			break;
		case 3: //Destination is beyond the edge
			return changeScreen(dir, world);
			break;
		case 4: //Destination is an NPC
			if(name == "Player")
			{
				world->speak(x_, y_);
			}
			return false;
			break;
	}
	
	return false;
}

bool NPC::move(char dir, World* world) //(n)orth, (s)outh, (w)est, (e)ast
{
	int x_ = x;
	int y_ = y;
	switch(dir)
	{
		case 'n':
			--y_;
			break;
		case 's':
			++y_;
			break;
		case 'w':
			--x_;
			break;
		case 'e':
			++x_;
			break;
	}
	
	if(x_ < t_x || x_ > b_x || y_ < t_y || y_ > b_y)
	{
		return false;
	}
	
	switch(world->passable(x_, y_))
	{
		case 0: //Destination is free,
			absoluteMove(x_, y_, world);
			x = x_;
			y = y_;
			return true;
			break;
			
		case 1: //Destination is occupied by movable object
			if(world->getScreen()->getObjects(x_, y_)->back()->move(dir, world))
			{
				absoluteMove(x_, y_, world);
				x = x_;
				y = y_;
				return true;
			}
			else
			{
				return false;
			}
			break;
		default: //Destination is blocked
			return false;
			break;
	}
	
	return false;
}

int Object::getY()
{
	return y;
}

void Object::absoluteMove(int x_, int y_, World* world)
{
	Object* temp;
	temp = world->getScreen()->getObjects(x, y)->back();
	world->getScreen()->getObjects(x, y)->pop_back();
	world->getScreen()->getObjects(x_, y_)->push_back(temp);
	x = x_;
	y = y_;
}

int Object::getID()
{
	return ID;
}

std::string Object::getName()
{
	return name;
}


Player::Player(int x_in, int y_in) : Object(x_in, y_in, 0, "Player", PLAYER_IMAGE, false, true, false) {}

int Object::getX()
{
	return x;
}

bool Player::changeScreen(char dir, World* world)
{
		switch(dir)
	{
		case 'n':
			absoluteMove(x, y+(MAP_H-1), world);
			break;
		case 's':
			absoluteMove(x, y-(MAP_H-1), world);
			break;
		case 'w':
			absoluteMove(x+(MAP_W-1), y, world);
			break;
		case 'e':
			absoluteMove(x-(MAP_W-1), y, world);
			break;
	}
	world->move(dir);
	return true;
}


NPC::NPC(int x_in, int y_in, int bx_in, int by_in, int ID_in, std::string name, std::string sprite_img)
	:  Object(x_in, y_in, ID_in, name, sprite_img, false, true, false), t_x(x_in), t_y(y_in), b_x(bx_in), b_y(by_in)
{
	std::fstream npc_file;
	npc_file.open(("objects/npcs/"+name+".npc").c_str());
	if(!npc_file)
	{
		std::cerr << "Couldn't find npc file for " << name << "." << std::endl;
	}
	else
	{
		bool multi;
		int state_number;
		while(npc_file.peek() != EOF)
		{
			multi = false;
			npc_file >> state_number;
			npc_file.ignore(1000, '\n');
			std::string temp_read = "";
			std::vector<std::string> temp_vector;
			while(temp_read != "---")
			{
				if(temp_read != "")
				{
					multi = true;
				}
				temp_read = "";
				temp_vector.erase(temp_vector.begin(), temp_vector.end());
				if(multi)
				{
					npc_file.ignore(1000, '\n');
				}
				while(npc_file.peek() != '\n')
				{
					npc_file >> temp_read;
					temp_vector.push_back(temp_read);
				}
				temp_vector.push_back("*DELIMITER*");
				npc_file.ignore(1000, '\n');
				while(npc_file.peek() != '\n')
				{
					std::getline(npc_file, temp_read);
					if(temp_read != "---")
					{
						temp_vector.push_back(temp_read);
					}
					else
					{
						break;
					}
				}
				dialogue_map[state_number].first = multi;
				dialogue_map[state_number].second.push_back(temp_vector);
			}
		}
	}
	
}

std::vector<std::string>* NPC::getDialogue(World* world)
{
	if(dialogue_map[world->getState(ID)].first)
	{
		for(std::vector<std::vector<std::string>>::iterator o_it = dialogue_map[world->getState(ID)].second.begin();
			o_it != dialogue_map[world->getState(ID)].second.end(); ++o_it)
		{
			for(std::vector<std::string>::iterator i_it = o_it->begin(); i_it != o_it->end(); ++i_it)
			{
				if(*i_it == "*DELIMITER*")
				{
					std::vector<std::string>* temp = new std::vector<std::string>(*o_it);
					temp->erase(temp->begin(),temp->begin()+(i_it - o_it->begin())+1);
					return temp;
				}
				else
				{
					std::stringstream ss(*i_it);
					char what;
					int check_ID, check_state;
					int map_x, map_y;
					ss >> what;
					ss.ignore(1000, ':');
					
					if(what == 'D')
					{
					}
					else if(what == 'I')
					{
						ss >> check_ID;
						ss.ignore(1000, ':');
						ss >> check_state;
						if(world->getState(check_ID) != check_state)
						{
							break;
						}
					}
					else if(what == 'M')
					{
						ss >> map_x;
						ss.ignore(1000, ':');
						ss >> map_y;
						if(world->getX() != map_x || world->getY() != map_y)
						{
							break;
						}
					}
					else
					{
						std::cerr << "False line in npc file." << std::endl;
						break;
					}
				}
			}
		}
	}
	else
	{
		std::vector<std::string>* temp = new std::vector<std::string>;
		*temp = dialogue_map[world->getState(ID)].second.back();
		temp->erase(temp->begin(),temp->begin()+2);
		return temp;
	}
	
	return nullptr;
}

