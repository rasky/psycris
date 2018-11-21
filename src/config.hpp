#pragma once
#include <string>

namespace psycris {
	struct config {
		bool verbose = false;
		std::string bios_file;
		size_t ticks = 10000;
	};

	extern config cfg;

	void parse_cmdline(int argc, char* argv[]);
}