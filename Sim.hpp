#ifndef SIM_HPP
#define SIM_HPP

#include "Vector.hpp"

#include <Box2D/Box2D.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>

const float SimTick = 0.01f;

class BlockHeader {
public:
	BlockHeader() : next(NULL), prev(NULL), size(0) {
	}
	BlockHeader *next;
	BlockHeader *prev;
	size_t size;
	void *mem() {
		assert(sizeof(BlockHeader) % sizeof(void *) == 0);
		assert(((uint8_t *)this + sizeof(BlockHeader)) == (uint8_t *)(this + 1));
		return reinterpret_cast< void * >(this + 1);
	}
	const void *mem() const {
		assert(sizeof(BlockHeader) % sizeof(void *) == 0);
		assert(((const uint8_t *)this + sizeof(BlockHeader)) == (const uint8_t *)(this + 1));
		return reinterpret_cast< const void * >(this + 1);
	}

	static BlockHeader *from_mem(void *mem) {
		return reinterpret_cast< BlockHeader * >(mem) - 1;
	}
};

class OtherState {
public:
	OtherState() : current_tick(0) {
	}

	unsigned int current_tick;

};

class Control {
public:
	Control() : signal(NONE) {
	}
	int signal;
	enum {
		NONE,
		LEFT,
		RIGHT,
	};
};

typedef std::unordered_map< size_t, BlockHeader * > BlockMap;

class SimMem;

class Sim : public OtherState {
public:
	Sim();
	~Sim();

	b2World *world;

	std::vector< b2Body * > stuff;

	b2Body *wheel;
	b2RevoluteJoint *pin;
	

	void tick(Control const &control);

	void draw() const;

	void save(SimMem &mem, SimMem *rel = NULL) const;
	void load(SimMem const &mem);

//Memory allocation from Box2D is wrapped back to these calls so that memory can be tracked:
	void *b2Alloc(int32 size);
	void b2Free(void *mem);

	BlockHeader *last_block;
	BlockMap free_lists;
};


#endif //SIM_HPP
