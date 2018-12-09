#include "config.hpp"
#include "cpu/cpu.hpp"
#include "loader.hpp"
#include "logging.hpp"
#include "psx.hpp"

#include <cassert>
#include <fmt/format.h>
#include <fstream>

// static uint8_t bios[512 * 1024];
// static uint8_t ram[2 * 1024 * 1024];

int main(int argc, char* argv[]) {
	using psycris::cfg;

	psycris::parse_cmdline(argc, argv);
	psycris::init_logging(cfg.verbose);

	psycris::psx board;

	// cpu::data_bus bus(bios, ram);
	// cpu::mips cpu(&bus);

	std::ifstream f(cfg.bios_file, std::ios::binary | std::ios::in);
	psycris::load_bios(f, board.rom.memory);

	board.cpu.run(cfg.ticks);
	fmt::print("run out of ticks\n");
}