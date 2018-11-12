#include "logging.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace psycris {
	std::shared_ptr<spdlog::logger> log;

	void init_logging() {
		log = spdlog::stdout_color_st(logger_name);
		log->set_pattern("[%^%l%$] %v");
	}
}