#include "CPWorld.hpp"
#include "Graphics.hpp"

const unsigned int MaxTick = 100 * 60 * 5; //five minutes will be recorded until we stop.

CPWorld::CPWorld() : time_acc(0.0f) {
	start = std::make_shared< SimMem >();
	sim.save(*start);
}

CPWorld::~CPWorld() {
}

void CPWorld::handle_event(SDL_Event const &e) {
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
		this->quit();
		return;
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

	time_acc += elapsed;
	while (time_acc > 0.0f) {
		time_acc -= SimTick;
		if (sim.current_tick < MaxTick) {
			assert(sim.current_tick == states.size());
			states.push_back(SimPt());
			states.back().control = cur_control;
			sim.tick(states.back().control);
			sim.save(*states.back().mem);
		}
	}
}

void CPWorld::draw() {
	Vector3f camera = make_vector(0.0f, 0.0f, 6.0f);

	glClearColor(0.9f, 0.9f, 0.9f, 0.9f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(1.0f / (camera.z * aspectf()), 1.0f / camera.z, 1.0f);
	glTranslatef(-camera.x, -camera.y, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	sim.draw();
}
