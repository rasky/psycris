#pragma once
#include "../bus.hpp"
#include "../mmap_device.hpp"

namespace cpu {
	class cop0;
}

namespace psycris::hw {
	class interrupt_control : public mmap_device<interrupt_control, 8> {
	  public:
		interrupt_control(gsl::span<uint8_t, size> buffer, cpu::cop0& cop);

	  public:
		using i_stat = data_reg<0>;
		using i_mask = data_reg<4>;

		void wcb(i_stat, uint32_t new_value, uint32_t old_value);

	  private:
		cpu::cop0* cop0;
	};
}