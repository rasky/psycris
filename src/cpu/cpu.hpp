#pragma once
#include <array>
#include <cstdint>

namespace cpu {
	class memory_bus {
	  public:
		template <typename T>
		T read(uint32_t addr) {
			return {};
		}

		template <typename T>
		void write(uint32_t addr, T val) {
		}
	};

	class mips {
	  public:
		static const uint32_t reset_vector = 0x1FC00000;

	  public:
		std::array<uint32_t, 32> regs;
		uint32_t pc;
		uint64_t clock;

		memory_bus* bus;

		constexpr uint32_t& operator[](uint8_t ix) {
			return regs[ix];
		}

	  public:
		mips(memory_bus*);

		void reset();

		void run(uint64_t until);
	};
}