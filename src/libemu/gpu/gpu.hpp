#pragma once
#include "../devices/mmap_device.hpp"
#include "../fixed_fifo.hpp"
#include "decoder.hpp"

#include <optional>

namespace psycris::gpu {
	using hw::data_reg;
	using hw::mmap_device;
	using hw::rw_buffers;

	class cxd;

	class controller : public mmap_device<controller, 8, rw_buffers::distinct> {
	  public:
		static constexpr char const* device_name = "GPU controller";

	  private:
		controller(gsl::span<uint8_t, addressable_size> r, gsl::span<uint8_t, addressable_size> w, cxd& gpu);

	  private:
		using gp0 = data_reg<0, 4>;
		using gp1 = data_reg<4, 4>;

		friend mmap_device;

		void wcb(gp0, uint32_t, uint32_t);
		void wcb(gp1, uint32_t, uint32_t);

	  private:
		friend cxd;

		cxd* gpu;
	};

	class cxd {
	  public:
		static constexpr char const* device_name = "GPU CXD???";

		static cxd build(gsl::span<uint8_t, controller::size> buffer);

	  private:
		cxd();

	  public:
		controller& ctrl_device() const;

		void run(uint64_t until);

		uint64_t ticks() const;

	  public:
		bool push_gp0(uint32_t);

		bool push_gp1(uint32_t);

	  private:
		std::optional<decoder> pop_gp0();
		std::optional<decoder> pop_gp1();

	  private:
		std::unique_ptr<controller> ctrl;

		uint64_t clock;

		fixed_fifo<uint32_t, 16> gp0_commands;
		fixed_fifo<uint32_t, 16> gp1_commands;

	  private:
		struct cxd_ins;

		uint32_t draw_mode;
	};
}