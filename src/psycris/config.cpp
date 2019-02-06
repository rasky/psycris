#include "config.hpp"
#include <CLI/CLI.hpp>
#include <cstdlib>

namespace psycris {
	config cfg;

	void parse_cmdline(int argc, char* argv[]) {
		CLI::App app("psycris");

		app.add_flag("--verbose", cfg.verbose, "be verbose");
		app.add_option("--ticks,-t", cfg.ticks, "number of CPU ticks to simulate");
		app.add_flag("--dump-on-exit", cfg.dump_on_exit, "dump board state on exit");
		app.add_flag("--restore",
		             [&](size_t) { cfg.mode = cfg.restore; },
		             "Restore the psx state from the input file. The input file is the result of a previous dump.");

		app.add_option("input_file", cfg.input_file, "the bios to load") //
		    ->required()                                                 //
		    ->check(CLI::ExistingFile);                                  //

		try {
			app.parse(argc, argv);
		} catch (const CLI::ParseError& e) {
			std::exit(app.exit(e));
		}
	}
}