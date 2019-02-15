#include "debug.hpp"
#include "psx.hpp"

namespace psycris::dbg {
	psycris::psx* board;

	uint64_t cpu_ticks() { return board->cpu.ticks(); }
}