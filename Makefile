OBJS = TileSystem.cc Objects.cc main.cc

CXXFLAGS = -Wall -Wextra -std=c++11 -Wno-unused-parameter

SDL_FLAGS = `sdl-config --libs --cflags` -lSDL_image -lSDL_ttf

COMPILER = g++

all: $(OBJS)
	$(COMPILER) $(CXXFLAGS) $(OBJS) $(SDL_FLAGS) -o game