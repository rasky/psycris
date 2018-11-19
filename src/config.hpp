#pragma once
#include <string>

namespace psycris {
	struct config {
		bool verbose = false;

        enum run_mode {
            bios,
            exe,
        };
        run_mode input_mode = run_mode::exe;

        std::string input_file;
	};

	extern config cfg;

	void parse_cmdline(int argc, char* argv[]);
}