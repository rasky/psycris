#include "gpu.hpp"
#include "../bitmask.hpp"
#include "../logging.hpp"

#include "disassembly.hpp"

namespace psycris::gpu {
	using psycris::log;

	namespace {}

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

	struct cxd::cxd_ins {
		cxd& gpu;

		using gst_mask = bit_mask<class gpustat_>;
		static constexpr gst_mask texture_x_base{0x0000'000f};
		static constexpr gst_mask texture_y_base{0x0000'0010};
		static constexpr gst_mask semi_transparency{0x0000'0060};
		static constexpr gst_mask texture_page_color{0x0000'0180};
		static constexpr gst_mask dither{0x0000'0200};
		static constexpr gst_mask drawing_enable{0x0000'0400};
		static constexpr gst_mask texture_disable{0x0000'8000};

		void draw_mode_settings(uint32_t ins) const {
			draw_mode_decoder d{ins};

			uint32_t stat = gpustat();

			texture_x_base(stat) = d.x_base();
			texture_y_base(stat) = d.y_base();
			semi_transparency(stat) = static_cast<uint8_t>(d.stm());
			texture_page_color(stat) = static_cast<uint8_t>(d.tpc());
			dither(stat) = d.dither();
			drawing_enable(stat) = d.drawing_enabled();
			texture_disable(stat) = d.texture_disabled();

			set_gpustat(stat);
		}

		uint32_t gpustat() const { return gpu.ctrl->read<controller::gp1, hw::buffer::r>(); }

		void set_gpustat(uint32_t v) const { gpu.ctrl->write<controller::gp1, hw::buffer::r>(v); }
	};

	cxd::cxd() : ctrl{nullptr}, clock{0} {}

	controller& cxd::ctrl_device() const { return *ctrl; }

	uint64_t cxd::ticks() const { return clock; }

	void cxd::run(uint64_t until) {
		cxd_ins exec{*this};

		while (clock < until) {
			auto decoded = pop_gp0();
			if (!decoded) {
				// no more instructions; we can break the loop and "jump" directly to the last tick
				break;
			}

			switch (decoded->opcode()) {
			case 0xe1:
				log->warn("RUN TEX!!!!");
				exec.draw_mode_settings(decoded->ins);
				break;
			default:
				log->critical("unimplemented GPU command");
			}

			clock++;
		}
		clock = until;
	}

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