#pragma once
#include "bus.hpp"
#include "decoder.hpp"
#include <array>
#include <cstdint>

namespace cpu {
	class mips {
	  public:
		static const uint32_t reset_vector = 0x1FC0'0000;

	  public:
		std::array<uint32_t, 32> regs;
		uint32_t pc;
		uint64_t clock;

		data_bus* bus;

		constexpr uint32_t& operator[](uint8_t ix) {
			return regs[ix];
		}

	  public:
		mips(data_bus*);

		void reset();

		void run(uint64_t until);

	  private:
		uint32_t& rs();
		uint32_t& rt();
		uint32_t& rd();

		decoder dec;
	};
}