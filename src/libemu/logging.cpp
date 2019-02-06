#include "logging.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace psycris {
	std::shared_ptr<spdlog::logger> log;

	void init_logging(bool verbose) {
		log = spdlog::stdout_color_st(logger_name);
		log->set_pattern("[%^%l%$] %v");

		if (verbose) {
			log->set_level(spdlog::level::level_enum::trace);
		}
	}
}