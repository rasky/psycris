#pragma once
#include "../hw/mmap_device.hpp"

namespace psycris::gpu {
	using hw::data_reg;
	using hw::mmap_device;

	class gpu_controller : public mmap_device<gpu_controller, 8> {
	  public:
		static constexpr char const* device_name = "GPU controller";

		gpu_controller(gsl::span<uint8_t, size> buffer);

	  private:
		using gp0 = data_reg<0, 4>;
		using gp1 = data_reg<4, 4>;

		friend mmap_device;

		void wcb(gp0, uint32_t, uint32_t);
		void wcb(gp1, uint32_t, uint32_t);
	};
}