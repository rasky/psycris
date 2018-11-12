#include "config.hpp"
#include "cpu/cpu.hpp"
#include "logging.hpp"

#include <cassert>
#include <fstream>
#include <iostream>

static uint8_t bios[512 * 1024];
static uint8_t ram[2 * 1024 * 1024];

int main(int argc, char* argv[]) {
	psycris::parse_cmdline(argc, argv);
	psycris::init_logging(psycris::cfg.verbose);

	std::ifstream f("bios/SCPH7003.bin", std::ios::binary | std::ios::in);
	if (!f.read((char*)&bios[0], sizeof(bios))) {
		assert(!"cannot read BIOS");
	}

	cpu::data_bus bus(bios, ram);
	cpu::mips cpu(&bus);

	cpu.run(15000);
}