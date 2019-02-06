#include "scheduler.hpp"
#include "cpu/cpu.hpp"
#include "gpu/gpu.hpp"

namespace psycris {
	scheduler::scheduler(cpu::mips& c, gpu::gpu& g) : _cpu{&c}, _gpu{&g} {}

	void scheduler::run() {
		uint64_t gpu_target = _gpu->ticks();
		uint64_t cpu_target = _cpu->ticks();

		uint64_t gpu_cycles_scanlines = 3406; // 3413 when in NTSC
		uint64_t cpu_cycles_scanlines = gpu_cycles_scanlines * 7 / 11;
		while (true) {
			gpu_target += gpu_cycles_scanlines;
			_gpu->run(gpu_target);

			cpu_target += cpu_cycles_scanlines;
			_cpu->run(cpu_target);
		}
	}
}