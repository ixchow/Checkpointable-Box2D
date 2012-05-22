
# Let me just mention that I use jam (ft-jam, actually) as my make system
# almost all the time, but I figured that a quick-to-compile example shouldn't
# require folks to go and get a new (vastly superior, mind you) build system.

UNAME = $(shell uname -s)

ifeq ($(UNAME),Darwin)
	CXX = g++-mp-4.6 -Wall -Werror -g --std=c++0x
else
	CXX = g++ -Wall -Werror -g --std=c++0x
endif
CXX_FLAGS = `sdl-config --cflags` -I./box2d/Box2D
ifeq ($(UNAME),Darwin)
	LD_FLAGS = `sdl-config --libs` -framework OpenGL -L./box2d/Box2D/Build/Box2D -lBox2D
else
	LD_FLAGS = `sdl-config --libs` -lGL -lGLU -L./box2d/Box2D/Build/Box2D -lBox2D
endif

HEADERS = Box.hpp CPWorld.hpp Graphics.hpp Screen.hpp Sim.hpp SimMem.hpp Vector.hpp
SOURCES = CPWorld.cpp Graphics.cpp Screen.cpp Sim.cpp SimMem.cpp main.cpp

main : $(SOURCES) $(HEADERS)
	$(CXX) $(CXX_FLAGS) $(SOURCES) $(LD_FLAGS) -o main
