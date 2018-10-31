#include "cpu/cpu.hpp"
#include <cassert>
#include <fstream>

static uint8_t bios[512 * 1024];
static uint8_t ram[2 * 1024 * 1024];

int main(int argc, char* argv[]) {
	std::ifstream f("bios/SCPH7003.bin", std::ios::binary | std::ios::in);
	if (!f.read((char*)&bios[0], sizeof(bios))) {
		assert(!"cannot read BIOS");
	}

	cpu::memory_bus bus;
	cpu::mips cpu(&bus);

	cpu.run(1000);
}