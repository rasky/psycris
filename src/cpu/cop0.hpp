#pragma once
#include <array>

namespace cpu {
	class cop0 {
	  public:
		cop0() = default;

		void reset() {
			regs.fill(0);
		}

	  public:
		std::array<uint32_t, 32> regs;
	};
}