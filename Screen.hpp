#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <SDL.h>

#include "Vector.hpp"
#include "Box.hpp"

/*
 * 'Screen' is a base class for interactive OpenGL/SDL apps.
 */

class Screen {
public:
	Screen(); //sets up screen to occupy, well, the entire screen.
	virtual ~Screen() { }

	//update the current world
	virtual void update(float elapsed_time) { }
	
	//handle the given sdl input event
	virtual void handle_event(SDL_Event const &event) { }
	
	//draw the current game world.
	virtual void draw() { }

	//internally, a screen is [-aspectf(),aspectf()]x[-1,1].
	float aspectf() const;
	double aspect() const;

	void update_screen_mouse_relative(SDL_MouseMotionEvent const &motion, Vector2f &mouse);
	void update_screen_mouse_absolute(SDL_MouseMotionEvent const &motion, Vector2f &mouse);
	void update_screen_mouse_relative(SDL_MouseMotionEvent const &motion, Vector2d &mouse);
	void update_screen_mouse_absolute(SDL_MouseMotionEvent const &motion, Vector2d &mouse);

	void run(); //actually runs the screen. call this->quit() to stop.
	void quit();

	//if a frame took less than this amount of time to render, wait.
	// (This prevents the cpu from being hogged.)
	static Uint32 min_ms_per_frame;
	
	//if a frame took more than this amount of time to render, pretend it
	//didn't (results in a game slow-down)
	static Uint32 max_ms_per_frame;

protected:
	Box2f pixel_extents; //size in pixels.
	Box2f extents; //size and location in ambient units.
	bool quit_flag;
};

#endif //SCREEN_HPP
