#include "Graphics.hpp"

#include <iostream>
#include <list>
#include <utility>
#include <assert.h>

using std::string;

using std::cerr;
using std::endl;

using std::list;
using std::pair;
using std::make_pair;

namespace Graphics {

Vector2ui screen_size = make_vector(800U, 600U);
bool fullscreen = false;

namespace { //private-type variables.
SDL_Surface *surface = NULL;
bool sub_inited = false;
bool inited = false;
} //end anon namespace

bool init(int flags) {
	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0) {
		sub_inited = true;
		
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8);
		if (flags & NEED_ACCUM) {
			SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 16);
			SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 16);
			SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 16);
		}
		//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8);
		//SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);
		if (flags & NEED_STENCIL) {
			SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);
		}
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
		if (flags & WANT_VSYNC) {
			#ifdef SDL_GL_SWAP_CONTROL
			SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1);
			#else
			cerr << "Using a version of SDL that doesn't support VSYNC." << endl;
			#endif
		}
		if (flags & WANT_MULTISAMPLE4) {
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );
		}
		if (flags & WANT_MULTISAMPLE8) {
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8 );
		}
		if (flags & WANT_MULTISAMPLE16) {
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 16 );
		}

		if (fullscreen) {
			surface = SDL_SetVideoMode(screen_size.x, screen_size.y, 24, SDL_OPENGL | SDL_FULLSCREEN);
		} else {
			surface = SDL_SetVideoMode(screen_size.x, screen_size.y, 24, SDL_OPENGL);
		}

		if (surface) {
			inited = true;
		} else {
			cerr << "Could not create video surface: " << SDL_GetError() << endl;
		}
	} else {
		cerr << "Could not init video: " << SDL_GetError() << endl;
	}

	if (sub_inited && !inited) {
		deinit();
	}

	return inited;

}

void deinit() {
	if (sub_inited) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		sub_inited = false;
		inited = false;
		surface = NULL;
	}
}

void gl_errors(string const &where) {
	GLuint err;
	while ((err = glGetError()) != GL_NO_ERROR) {
	cerr << "(in " << where << ") OpenGL error #" << err
	     << ": " << gluErrorString(err) << endl;
	}
}

}
