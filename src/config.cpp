#include "config.hpp"
#include <CLI/CLI.hpp>
#include <cstdlib>

namespace psycris {
	config cfg;

	void parse_cmdline(int argc, char* argv[]) {
		CLI::App app("psycris");

		app.add_flag("--verbose", cfg.verbose, "be verbose");
		app.add_flag("--bios",
		             [&](size_t) { cfg.input_mode = config::bios; },
		             "use if the input file is a bios, the default is PSX-EXE");

		app.add_option("input_file", cfg.input_file, "execute this file") //
		    ->required()                                                  //
		    ->check(CLI::ExistingFile);                                   //

		try {
			app.parse(argc, argv);
		} catch (const CLI::ParseError& e) {
			std::exit(app.exit(e));
		}
	}
}