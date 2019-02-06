#include "gpu.hpp"
#include "../logging.hpp"
#include "disassembly.hpp"

namespace psycris::gpu {

	controller::controller(gsl::span<uint8_t, size> buffer) : mmap_device{buffer, gp0{}, gp1{}} {}

	void controller::wcb(gp0, uint32_t new_value, uint32_t) {
		psycris::log->info("GP0 command {:>8x} {}", new_value, disassembly(gpu_port::gp0, new_value));
	}

	void controller::wcb(gp1, uint32_t new_value, uint32_t) {
		psycris::log->info("GP1 command {:>8x} {}", new_value, disassembly(gpu_port::gp1, new_value));
	}

	using clock_t = data_reg<0, 4>;

	cxd::cxd(gsl::span<uint8_t, size> buffer) : mmap_device{buffer, clock_t{}} {}

	uint64_t cxd::ticks() const { return read<clock_t>(); }

	void cxd::run(uint64_t until) { write<clock_t>(until); }
}