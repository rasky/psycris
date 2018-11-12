#pragma once

namespace psycris {
	struct config {
		bool verbose = false;
	};

	extern config cfg;

	void parse_cmdline(int argc, char* argv[]);
}