#include "config.hpp"
#include <CLI/CLI.hpp>
#include <cstdlib>

namespace psycris {
	config cfg;

	void parse_cmdline(int argc, char* argv[]) {
		CLI::App app("psycris");
		app.add_flag("--verbose", cfg.verbose, "be verbose");
		try {
			app.parse(argc, argv);
		} catch (const CLI::ParseError& e) {
			std::exit(app.exit(e));
		}
	}
}