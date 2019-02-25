#include "debug.hpp"
#include "psx.hpp"

namespace psycris::dbg {
	psycris::psx* board;

	uint64_t cpu_ticks() { return board ? board->cpu.ticks() : 0; }

	uint32_t cpu_pc() { return board ? board->cpu.program_counter() : 0; }
}
