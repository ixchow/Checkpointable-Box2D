#include "Graphics.hpp"
#include "CPWorld.hpp"

#include <iostream>

using std::cerr;
using std::endl;

int main(int argc, char **argv) {

	Graphics::screen_size = make_vector(800U, 600U);

	if (SDL_Init(SDL_INIT_TIMER) != 0) {
		cerr << "Could not initialize sdl: " << SDL_GetError() << endl;
		return 1;
	}

	if (!Graphics::init()) {
		cerr << "Could not initialize graphics, not continuing." << endl;
		exit(1);
	}

	//Some quick basic GL state:

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	CPWorld world;

	world.run();

	Graphics::deinit();

	SDL_Quit();

	return 0;
}
