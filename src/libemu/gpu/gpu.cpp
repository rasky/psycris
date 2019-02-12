#include "gpu.hpp"
#include "../logging.hpp"
#include "disassembly.hpp"

namespace psycris::gpu {
	using psycris::log;

	controller::controller(gsl::span<uint8_t, addressable_size> r, gsl::span<uint8_t, addressable_size> w, cxd& gpu)
	    : mmap_device{r, w, gp0{}, gp1{}}, gpu{&gpu} {}

	void controller::wcb(gp0, uint32_t new_value, uint32_t) {
		psycris::log->info("GP0 command {:>8x} {}", new_value, disassembly(gpu_port::gp0, new_value));
		if (!gpu->push_gp0(new_value)) {
			log->critical("gpu fifo full, command lost!");
		}
	}

	void controller::wcb(gp1, uint32_t new_value, uint32_t) {
		psycris::log->info("GP1 command {:>8x} {}", new_value, disassembly(gpu_port::gp1, new_value));
	}

	cxd cxd::build(gsl::span<uint8_t, controller::size> buffer) {
		cxd gpu;
		auto r = buffer.subspan(0, controller::addressable_size);
		auto w = buffer.subspan(controller::addressable_size);
		gpu.ctrl.reset(new controller(r, w, gpu));
		return gpu;
	}

	cxd::cxd() : ctrl{nullptr}, clock{0} {}

	controller& cxd::ctrl_device() const { return *ctrl; }

	uint64_t cxd::ticks() const { return clock; }

	void cxd::run(uint64_t until) { clock = until; }

	bool cxd::push_gp0(uint32_t command) {
		if (gp0_commands.full()) {
			psycris::log->error("GP0: commands queue full");
			return false;
		}
		psycris::log->info("GP0: new commands {}", disassembly(gpu_port::gp0, command));
		gp0_commands.push(command);
		return true;
	}

	std::optional<decoder> cxd::pop_gp0() {
		if (gp0_commands.empty()) {
			return {};
		}
		return decoder{gp0_commands.pop()};
	}
}