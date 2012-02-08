#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "Vector.hpp"

#include <SDL.h>
#include <SDL_opengl.h>
#include <string>

namespace Graphics {
	//these can be set up before calling init()
	extern Vector2ui screen_size; //default 800x600
	extern bool fullscreen; //default false

	//convenience function for screen aspect ratio.
	inline double aspect() {
		return (double)screen_size.x / (double)screen_size.y;
	}
	inline float aspectf() {
		return (float)screen_size.x / (float)screen_size.y;
	}

	enum {
		NEED_ACCUM = 1 << 0,
		NEED_STENCIL = 1 << 1,
		WANT_VSYNC = 1 << 2,
		WANT_MULTISAMPLE4 = 1 << 3,
		WANT_MULTISAMPLE8 = 1 << 4,
		WANT_MULTISAMPLE16 = 1 << 5,
	};
	//set up graphics output. lets you know if it fails.
	bool init(int flags = 0);

	//tear down graphics output.
	void deinit();

	//helper function for gl code.
	//prints out all the current gl errors along with the 'where' string.
	void gl_errors(std::string const &where);

}


#endif
