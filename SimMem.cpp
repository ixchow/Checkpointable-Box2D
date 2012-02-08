#include "SimMem.hpp"

#include <iostream>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

StoredBlock::StoredBlock(BlockHeader *_from, const StoredBlock *rel) : from(_from), next(NULL), size(0), data(NULL), data_size(0), runs_count(0), runs(NULL) {
	assert(from);
	size = from->size;

	if (rel) {
		assert(rel->runs == NULL);
		assert(rel->from == from);
		vector< uint8_t > different_bytes;
		vector< uint16_t > run_lengths;
		const uint16_t MaxRun = 0xffff;
		run_lengths.push_back(0);
		uint8_t *ref_byte = rel->data;
		uint8_t *byte = reinterpret_cast< uint8_t * >(from->mem());
		for (unsigned int i = 0; i < size; ++i) {
			//first run is match, second is not match, third is match...
			bool match_run = (run_lengths.size() % 2);
			if (ref_byte[i] == byte[i]) {
				if (match_run) {
					if (run_lengths.back() == MaxRun) {
						run_lengths.push_back(0);
						run_lengths.push_back(1);
					} else {
						run_lengths.back() += 1;
					}
				} else {
					run_lengths.push_back(1);
				}
			} else {
				if (!match_run) {
					different_bytes.push_back(byte[i]);
					if (run_lengths.back() == MaxRun) {
						run_lengths.push_back(0);
						run_lengths.push_back(1);
					} else {
						run_lengths.back() += 1;
					}
				} else {
					//if we just had a match run of length 1, elide it:
					if (run_lengths.size() >= 2
					 && run_lengths.back() <= 2
					 && run_lengths[run_lengths.size()-2] <= MaxRun - 1 - run_lengths.back()) {
					 	uint16_t to_add = run_lengths.back();
						run_lengths.pop_back();
						run_lengths.back() += to_add + 1;
						if (to_add == 2) {
							different_bytes.push_back(byte[i-2]);
							different_bytes.push_back(byte[i-1]);
						} else if (to_add == 1) {
							different_bytes.push_back(byte[i-1]);
						} else {
							assert(0);
						}
						different_bytes.push_back(byte[i]);
					} else {
						//Otherwise, just proceed as expected:
						different_bytes.push_back(byte[i]);
						run_lengths.push_back(1);
					}
				}
			} //if (bytes match ) else
		} //for bytes, compute runs.
		if (sizeof(uint8_t) * different_bytes.size() + sizeof(uint16_t) * run_lengths.size() >= size) {
			cout << "Skipping compression for " << from << " rel " << rel << endl;
		} else {
			runs_count = run_lengths.size();
			runs = reinterpret_cast< uint16_t * >(malloc(sizeof(uint16_t) * run_lengths.size()));
			memcpy(runs, &(run_lengths[0]), sizeof(uint16_t) * run_lengths.size());
			data_size = different_bytes.size();
			data = reinterpret_cast< uint8_t * >(malloc(different_bytes.size()));
			memcpy(data, &(different_bytes[0]), different_bytes.size());
			//DEBUG:
			size_t expected_data_size = 0;
			for (unsigned int r = 0; r < runs_count; ++r) {
				if ((r + 1) % 2) {
				} else {
					expected_data_size += runs[r];
				}
			}
			assert(expected_data_size == data_size);
		}
	} //if (rel != NULL)

	//check runs to see if we decided to skip compression:
	if (runs == NULL) {
		data_size = size;
		data = reinterpret_cast< uint8_t * >(malloc(size));
		if (!data) {
			cerr << "Out of memory" << endl;
			exit(1);
		}
		memcpy(data, from->mem(), size);
	}
}

StoredBlock::~StoredBlock() {
	free(data);
	data = NULL;
	if (runs) {
		free(runs);
		runs = NULL;
	}
}

void StoredBlock::set_block_mem(const StoredBlock *rel) const {
	assert(from);
	assert(from->size == size);
	if (!runs) {
		memcpy(from->mem(), data, from->size);
		return;
	}
	assert(rel);
	assert(!rel->runs);
	//Time to delta-decompress!
	size_t data_ofs = 0;
	size_t to_ofs = 0;
	uint8_t const *rel_bytes = rel->data;
	uint8_t *to_bytes = reinterpret_cast< uint8_t * >(from->mem());
	for (unsigned int r = 0; r < runs_count; ++r) {
		if (runs[r] == 0) continue;
		assert(to_ofs + runs[r] <= size);
		if ((r + 1) % 2) {
			//match
			memcpy(to_bytes + to_ofs, rel_bytes + to_ofs, runs[r]);
		} else {
			//don't match
			assert(data_ofs + runs[r] <= data_size);
			memcpy(to_bytes + to_ofs, data + data_ofs, runs[r]);
			data_ofs += runs[r];
		}
		to_ofs += runs[r];
	}
	assert(to_ofs == size);
	assert(data_ofs == data_size);
}

void SimMem::free_blocks() {
	while (first_block) {
		StoredBlock *old = first_block;
		first_block = first_block->next;
		delete old;
	}
}
