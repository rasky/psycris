#include "config.hpp"
#include "loader.hpp"

#include "../libemu/logging.hpp"
#include "../libemu/psx.hpp"
#include "../libemu/scheduler.hpp"

#include "../libemu/debug.hpp"

#include <cstdlib>
#include <fstream>

namespace {

	void dump_on_exit() {
		// using psycris::log;

		// std::string filename = fmt::format("dump@{}", board.cpu.ticks());
		// log->info("saving the board dump on {}", filename);

		// std::ofstream dump_file(filename, std::ios::binary | std::ios_base::out | std::ios_base::trunc);
		// if (!dump_file) {
		// 	log->critical("cannot open the dump file for writing");
		// }

		// psycris::dump_board(dump_file, board);
	}
}

int main(int argc, char* argv[]) {
	using psycris::cfg;
	psycris::parse_cmdline(argc, argv);

	using psycris::log;
	psycris::init_logging(cfg.verbose);

	psycris::psx board;
	psycris::dbg::board = &board;

	log->info("PSX board. Total memory={}", psycris::psx::board::memory_size());
	if (cfg.dump_on_exit) {
		log->trace("dump on exit");
		std::atexit(dump_on_exit);
	}

	std::ifstream f(cfg.input_file, std::ios::binary | std::ios::in);
	if (!f) {
		log->critical("error opening {}", cfg.input_file);
		return 1;
	}

	if (cfg.mode == cfg.bios) {
		log->info("loading bios {}", cfg.input_file);
		psycris::load_bios(f, board.rom.writable_memory());
	} else {
		log->info("restoring from {}", cfg.input_file);
		psycris::restore_board(f, board);
	}

	psycris::scheduler sc(board.cpu, board.gpu);
	sc.run();
	// board.cpu.run(cfg.ticks);
	// fmt::print("run out of ticks\n");
}