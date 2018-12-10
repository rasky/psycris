#pragma once
#include "../cpu/bus.hpp"
#include "../cpu/cop0.hpp"
#include "memory_range.hpp"

#include <fmt/format.h>

namespace psycris::hw {
	class interrupt_control : public cpu::data_port, public memory_range<8> {
	  public:
		interrupt_control(gsl::span<uint8_t, size> buffer, cpu::cop0& cop0) : memory_range(buffer), _cop0(&cop0) {
		}

	  public:
		constexpr static uint32_t i_stat = 0;
		constexpr static uint32_t i_mask = 0;

		uint32_t read32(uint32_t offset) const override {
			return *(reinterpret_cast<uint32_t*>(&memory[offset]));
		}

		void write32(uint32_t offset, uint32_t val) override {
			uint32_t p = read32(i_stat);
			*(reinterpret_cast<uint32_t*>(&memory[offset])) = val;

			if (offset == i_stat) {
				p = (p ^ val) & read32(i_mask);
				if (p) {
					_cop0->interrupt_request();
				}
			}
		}

	  private:
		cpu::cop0* _cop0;
	};
}