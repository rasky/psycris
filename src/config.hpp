#pragma once
#include <string>

namespace psycris {
	struct config {
		bool verbose = false;

		std::string input_file;

		enum start_mode {
			bios,
			restore,
		};
		start_mode mode = bios;

		size_t ticks = 10000;
		bool dump_on_exit = false;
	};

	extern config cfg;

	void parse_cmdline(int argc, char* argv[]);
}