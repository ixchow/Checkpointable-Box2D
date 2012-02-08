#ifndef SIMMEM_HPP
#define SIMMEM_HPP

#include "Sim.hpp"

class StoredBlock {
public:
	StoredBlock(BlockHeader *_from, const StoredBlock *rel = NULL);
	~StoredBlock();
	void set_block_mem(const StoredBlock *rel = NULL) const; //set the mem() part of from to what we're storing.
	BlockHeader *from;
	StoredBlock *next;
	size_t size; //for sanity checking.
	uint8_t *data;
	size_t data_size; //for later assert()-ing
	//for delta compression:
	size_t runs_count;
	uint16_t *runs;
	//DEBUG: for memory profiling stuff:
	size_t estimate_size() {
		if (runs) {
			size_t mem = sizeof(StoredBlock) + runs_count * sizeof(uint16_t);
			for (unsigned int r = 0; r < runs_count; ++r) {
				if ((r+1) % 2) {
					//match run.
				} else {
					mem += runs[r];
				}
			}
			return mem;
		} else {
			return sizeof(StoredBlock) + size;
		}
	}
};

class SimMem {
public:
	SimMem() : first_block(NULL), rel(NULL) {
	}
	~SimMem() {
		free_blocks();
	}
	void free_blocks();
	StoredBlock *first_block;
	const SimMem *rel;
	OtherState state;
};


#endif //SIMMEM_HPP
