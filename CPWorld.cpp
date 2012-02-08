#include "CPWorld.hpp"
#include "Graphics.hpp"

using std::shared_ptr;
using std::make_shared;
using std::vector;

const unsigned int MaxTick = 100 * 60 * 2; //two minutes will be recorded until we stop.

CPWorld::CPWorld() : time_acc(0.0f), alt_acc(0.0f) {
	start = make_shared< SimMem >();
	sim.save(*start);
}

CPWorld::~CPWorld() {
}

void CPWorld::handle_event(SDL_Event const &e) {
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
		this->quit();
		return;
	}
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
		others.push_back(make_shared< SimMem >());
		sim.load(*start);
		sim.save(*others.back());
		if (states.empty()) {
			sim.load(*start);
		} else {
			sim.load(*states.back().mem);
		}
	}
}


void CPWorld::update(float elapsed) {
	static Uint8 *keys = SDL_GetKeyState(NULL);

	Control cur_control;
	if (keys[SDLK_LEFT] && keys[SDLK_RIGHT]) {
		cur_control.signal = Control::NONE;
	} else if (keys[SDLK_LEFT]) {
		cur_control.signal = Control::LEFT;
	} else if (keys[SDLK_RIGHT]) {
		cur_control.signal = Control::RIGHT;
	} else {
		cur_control.signal = Control::NONE;
	}

	float speed = 1.0f;
	if (keys[SDLK_a]) {
		speed = -2.0f;
	} else if (keys[SDLK_d]) {
		speed = 2.0f;
	}

	time_acc += elapsed * speed;
	if (time_acc > 0.0f) {
		while (time_acc > 0.0f) {
			time_acc -= SimTick;
			if (sim.current_tick < MaxTick) {
				assert(sim.current_tick == states.size());
				states.push_back(SimPt());
				states.back().control = cur_control;
				sim.tick(states.back().control);
				sim.save(*states.back().mem);
			} else {
				time_acc = 0.0f;
			}
		}
	} else {
		while (time_acc <= -SimTick) {
			time_acc += SimTick;
			if (!states.empty()) {
				states.pop_back();
			}
			if (!states.empty()) {
				sim.load(*states.back().mem);
			} else {
				sim.load(*start);
			}
		}
	}

	//Simulate others forward:
	alt_acc += elapsed;
	unsigned int other_ticks = 0;
	while (alt_acc > 0.0f) {
		alt_acc -= SimTick;
		++other_ticks;
	}

	for (vector< shared_ptr< SimMem > >::iterator o = others.begin(); o != others.end(); /* later */ ) {
		sim.load(**o);
		for (unsigned int i = 0; i < other_ticks; ++i) {
			if (sim.current_tick < states.size()) {
				sim.tick(states[sim.current_tick].control);
			}
		}
		sim.save(**o);
		if (sim.current_tick >= states.size()) {
			//this other state has caught up, delete it.
			o = others.erase(o);
		} else {
			++o;
		}
	}

	//load the current state again:
	if (states.empty()) {
		sim.load(*start);
	} else {
		sim.load(*states.back().mem);
	}

}

void CPWorld::draw() {
	Vector3f camera = make_vector(0.0f, 0.0f, 6.0f);

	glClearColor(0.9f, 0.9f, 0.9f, 0.9f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	//draw game world:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f / (camera.z * aspectf()), 1.0f / camera.z, 1.0f);
	glTranslatef(-camera.x, -camera.y, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	//draw other states:
	for (vector< shared_ptr< SimMem > >::iterator o = others.begin(); o != others.end(); ++o ) {
		sim.load(**o);
		sim.draw(0.5f);
	}

	//load the current state again:
	if (states.empty()) {
		sim.load(*start);
	} else {
		sim.load(*states.back().mem);
	}

	sim.draw();

	/* One way to use the save/load potential is to calculate various futures:
	for (unsigned int i = 0; i < 100; ++i) {
		Control control;
		if (!states.empty()) {
			control = states.back().control;
		}
		sim.tick(control);
		sim.draw(0.1f);
	}
	//reset sim, which we ran forward to draw future:
	if (!states.empty()) {
		sim.load(*states.back().mem);
	} else {
		sim.load(*start);
	}
	*/

	//draw overlay:
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f / aspectf(), 1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_LINE_LOOP);
	glColor3f(0.7f, 0.7f, 0.7f);
	glVertex2f(-1.0f, 0.95f);
	glVertex2f(-1.0f, 0.90f);
	glVertex2f( 1.0f, 0.90f);
	glVertex2f( 1.0f, 0.95f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.4f, 0.4f, 0.4f);
	glVertex2f(-1.0f, 0.95f);
	glVertex2f(-1.0f, 0.90f);
	glVertex2f(-1.0f + sim.current_tick / float(MaxTick), 0.90f);
	glVertex2f(-1.0f + sim.current_tick / float(MaxTick), 0.95f);
	glEnd();

}
