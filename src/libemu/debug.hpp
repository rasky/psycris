#pragma once
#include <cstdint>

namespace psycris {
	class psx;

	namespace dbg {
		extern psx* board;

		uint64_t cpu_ticks();

		uint32_t cpu_pc();
	}
}