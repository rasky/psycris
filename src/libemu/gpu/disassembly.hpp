#pragma once
#include <string>

namespace psycris::gpu {
	enum class gpu_port {
		gp0,
		gp1,
	};

	std::string disassembly(gpu_port port, uint32_t ins);
}