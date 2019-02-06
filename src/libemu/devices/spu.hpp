#pragma once
#include "mmap_device.hpp"

namespace psycris::hw {
	class spu : public mmap_device<spu, 512> {
	  public:
		static constexpr char const* device_name = "SPU";

		spu(gsl::span<uint8_t, size> buffer);

	  private:
		// The SPU has many many more registers; they are all described in the
		// cpp file. Here are defined only the ones needed to declare the spu class.

		// SPU Control and Status Register
		using spucnt = data_reg<426, 2>;
		using spustat = data_reg<430, 2>;

		friend mmap_device;

		void wcb(spucnt, uint32_t, uint32_t);
		void wcb(spustat, uint32_t, uint32_t);
	};
}
