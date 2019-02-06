#include "gpu.hpp"
#include "../logging.hpp"
#include "disassembly.hpp"

namespace psycris::gpu {

	gpu_controller::gpu_controller(gsl::span<uint8_t, size> buffer) : mmap_device{buffer, gp0{}, gp1{}} {}

	void gpu_controller::wcb(gp0, uint32_t new_value, uint32_t) {
		psycris::log->info("GP0 command {:>8x} {}", new_value, disassembly(gpu_port::gp0, new_value));
	}

	void gpu_controller::wcb(gp1, uint32_t new_value, uint32_t) {
		psycris::log->info("GP1 command {:>8x} {}", new_value, disassembly(gpu_port::gp1, new_value));
	}

	using clock_t = data_reg<0, 4>;

	gpu::gpu(gsl::span<uint8_t, size> buffer) : mmap_device{buffer, clock_t{}} {}

	uint64_t gpu::ticks() const { return read<clock_t>(); }

	void gpu::run(uint64_t until) { write<clock_t>(until); }
}