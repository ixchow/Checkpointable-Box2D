#include "Screen.hpp"
#include "Graphics.hpp"

#include <vector>

#include <cassert>

using std::vector;

Screen::Screen() {
	//occupies the entire screen:
	pixel_extents.max = make_vector< float >(Graphics::screen_size.x, Graphics::screen_size.y);
	pixel_extents.min = make_vector< float >(0,0);
	extents.max = make_vector(Graphics::aspectf(), 1.0f);
	extents.min =-make_vector(Graphics::aspectf(), 1.0f);
	quit_flag = false;
}

float Screen::aspectf() const {
	return extents.size().x / extents.size().y;
}

double Screen::aspect() const {
	return extents.size().x / extents.size().y;
}

void Screen::update_screen_mouse_relative(SDL_MouseMotionEvent const &motion, Vector2f &mouse) {
	mouse.x += motion.xrel / pixel_extents.size().x * 2.0f * aspectf();
	mouse.y -= motion.yrel / pixel_extents.size().y * 2.0f;
}

void Screen::update_screen_mouse_absolute(SDL_MouseMotionEvent const &motion, Vector2f &mouse) {
	mouse.x =  (motion.x - pixel_extents.min.x) / (pixel_extents.max.x - pixel_extents.min.x) * 2.0f * aspectf() - aspectf();
	mouse.y =  (motion.y - pixel_extents.min.y) / (pixel_extents.max.y - pixel_extents.min.y) *-2.0f + 1.0f;
}

void Screen::update_screen_mouse_relative(SDL_MouseMotionEvent const &motion, Vector2d &mouse) {
	mouse.x += motion.xrel / pixel_extents.size().x * 2.0 * aspect();
	mouse.y -= motion.yrel / pixel_extents.size().y * 2.0;
}

void Screen::update_screen_mouse_absolute(SDL_MouseMotionEvent const &motion, Vector2d &mouse) {
	mouse.x =  (motion.x - pixel_extents.min.x) / (pixel_extents.max.x - pixel_extents.min.x) * 2.0 * aspect() - aspect();
	mouse.y =  (motion.y - pixel_extents.min.y) / (pixel_extents.max.y - pixel_extents.min.y) *-2.0 + 1.0;
}

Uint32 Screen::min_ms_per_frame = 1000/100;
Uint32 Screen::max_ms_per_frame = 1000/15;

namespace {
bool quit_everything = false;
}

void Screen::run() {

	Uint32 env_t = SDL_GetTicks();
	Uint32 env_et = 0;
	SDL_Event event;

	while (!(quit_flag || quit_everything)) {

		//Find elapsed milliseconds, delaying and clamping as needed.
		env_et = SDL_GetTicks() - env_t;
		while (env_et < min_ms_per_frame) {
			SDL_Delay(min_ms_per_frame - env_et);
			env_et = SDL_GetTicks() - env_t;
		}
		env_t += env_et;

		if (env_et > max_ms_per_frame) env_et = max_ms_per_frame;

		//get and process any new events.
		while (!(quit_flag || quit_everything) && SDL_PollEvent(&event)) {
			handle_event(event);
			//add alt-escape as a sort of force-quit:
			if ((event.type == SDL_KEYDOWN
				&& event.key.keysym.sym == SDLK_ESCAPE
				&& (event.key.keysym.mod & KMOD_ALT))
			 || event.type == SDL_QUIT) {
			 	quit_everything = true;
			}
		}

		if (quit_flag || quit_everything) break;
	
		//update the world
		update(env_et / 1000.0f);

		if (quit_flag || quit_everything) break;

		//draw the world
		draw();

		//swap the drawn world to the front buffer
		SDL_GL_SwapBuffers();
	}
}

void Screen::quit() {
	quit_flag = true;
}
