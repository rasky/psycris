#include "spu.hpp"
#include "../../logging.hpp"

namespace psycris::hw {
	using psycris::log;

	spu::spu(gsl::span<uint8_t, size> buffer) : mmap_device{buffer} {}

	void spu::wcb(spustat, uint32_t, uint32_t) { log->warn("SPUSTAT should be R/O"); }
}