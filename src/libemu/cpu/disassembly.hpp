#pragma once
#include <array>
#include <string>

namespace psycris::cpu {
	std::string disassembly(uint32_t ins, uint32_t pc);

	struct reg_tracer {
		std::array<uint32_t, 32> traced = {};

		void trace(std::array<uint32_t, 32> const& regs);
	};
}