#include "interrupt_control.hpp"
#include "../../cpu/cop0.hpp"

namespace psycris::hw {
	interrupt_control::interrupt_control(gsl::span<uint8_t, size> buffer, cpu::cop0& cop)
	    : mmap_device(buffer, i_stat{}, i_mask{}), cop0(&cop) {}

	void interrupt_control::wcb(i_stat, uint32_t new_value, uint32_t old_value) {
		auto int_request = (old_value ^ new_value) & read<i_mask>();
		if (int_request) {
			cop0->interrupt_request();
		}
	}
}