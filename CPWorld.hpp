#ifndef CPWORLD_HPP
#define CPWORLD_HPP

#include "Screen.hpp"
#include "Sim.hpp"
#include "SimMem.hpp"

#include <memory>

//For each tick, we record the control applied to arrive at that tick and
// the simulation state resulting from this control:
class SimPt {
public:
	SimPt() : mem(std::make_shared< SimMem >()) {
	}
	Control control;
	std::shared_ptr< SimMem > mem;
};

class CPWorld : public Screen {
public:
	CPWorld();
	virtual ~CPWorld();
	virtual void handle_event(SDL_Event const &event);
	virtual void update(float elapsed);
	virtual void draw();

	Sim sim;
	float time_acc;

	std::shared_ptr< SimMem > start; //starting state of simulator
	std::vector< SimPt > states;
};


#endif //CPWORLD_HPP
