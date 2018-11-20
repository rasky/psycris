#include "config.hpp"
#include "cpu/cpu.hpp"
#include "loader.hpp"
#include "logging.hpp"

#include <cassert>
#include <fstream>
#include <iostream>

static uint8_t bios[512 * 1024];
static uint8_t ram[2 * 1024 * 1024];

int main(int argc, char* argv[]) {
	using psycris::cfg;

	psycris::parse_cmdline(argc, argv);
	psycris::init_logging(cfg.verbose);

	cpu::data_bus bus(bios, ram);
	cpu::mips cpu(&bus);

	std::ifstream f(cfg.input_file, std::ios::binary | std::ios::in);
	if (cfg.input_mode == psycris::config::bios) {
		psycris::load_bios(f, bus);
	} else {
		psycris::load_exe(f, bus);
	}

	cpu.run(cfg.ticks);
}