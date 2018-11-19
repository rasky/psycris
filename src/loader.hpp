#pragma once
#include "cpu/bus.hpp"
#include <istream>

namespace psycris {
	void load_bios(std::istream&, cpu::data_bus&);

	void load_exe(std::istream&, cpu::data_bus&);
}