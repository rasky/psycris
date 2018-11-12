#pragma once
#include <spdlog/spdlog.h>

namespace psycris {
	constexpr char const* logger_name = "psycris";

	extern std::shared_ptr<spdlog::logger> log;

	void init_logging(bool verbose);
}