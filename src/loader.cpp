#include "loader.hpp"
#include "logging.hpp"
#include <algorithm>
#include <cstdlib>

#include <fmt/format.h>

namespace {
	struct exe_header {
		// magic string "PS-X EXE"
		uint8_t ascii_id[8];
		uint8_t unused1[7];

		// initial value for the PC
		uint32_t pc;
		// initial value for the GP (R28)
		uint32_t gp;
		// where to put the exe (in RAM)
		uint32_t load_address;
		// exe_size must be a multiple of 2048
		uint32_t exe_size;
		uint32_t unused2[2];

		// start of the memory region to fill with zero
		uint32_t memfill_start;
		// size of the zerofilled memory
		uint32_t memfill_size;

		// initial value for SP (R29) **and** FP (R30)
		uint32_t sp_base;
		// offset for SP **and** FP (added to the above base)
		uint32_t sp_offset;

		// reserved for A(43h) Function
		uint8_t reserved[20];

		uint8_t ascii_marker[56];

		std::string_view id() const {
			return {reinterpret_cast<const char*>(&ascii_id), 8};
		}

		std::string_view marker() const {
			return {reinterpret_cast<const char*>(&ascii_marker), sizeof(ascii_marker)};
		}
	};

	// A minimal bios used to start an exe
	//
	// This bios expects three arguments (three words stored starting from
	// 0x1fc'1000):
	//
	// 0x1fc0000 the initial value for GP
	// 0x1fc0004 the initial value for SP and FP
	// 0x1fc0008 the initial value for PC
	uint32_t exe_bios[] = {
	    // LUI t0, 0x1fc0
	    0x3c08'1fc0,
	    // ORI t0, t0, 0x1000
	    0x3508'1000,
	    // LW gp, 0x0(t0)
	    0x8d1c'0000,
	    // LW sp, 0x4(t0)
	    0x8d1d'0004,
	    // LW fp, 0x4(t0)
	    0x8d1e'0004,
	    // LW t1, 0x8(t0)
	    0x8d09'0008,
	    // JR t1
	    0x0120'0008,
	    // noop (delay slot)
	    0x0,
	};

	void load_exe_bios(cpu::data_bus& bus, uint32_t pc, uint32_t gp, uint32_t sp) {
		uint32_t exe_args[] = {gp, sp, pc};

		auto args = bus.bios_slice(0x1fc0'1000, sizeof(exe_args));
		std::copy_n(reinterpret_cast<uint8_t*>(exe_bios), sizeof(exe_bios), bus.bios.data());
		std::copy_n(reinterpret_cast<uint8_t*>(exe_args), sizeof(exe_args), args.data());
	}
}

namespace psycris {
	using psycris::log;

	void load_bios(std::istream& in, cpu::data_bus& bus) {
		auto ptr = reinterpret_cast<char*>(bus.bios.data());
		if (!in.read(ptr, bus.bios.size())) {
			log->critical("cannot read BIOS");
			std::exit(2);
		}
	}

	void load_exe(std::istream& in, cpu::data_bus& bus) {
		exe_header hdr;

		if (!in.read(reinterpret_cast<char*>(&hdr), sizeof(hdr))) {
			log->critical("cannot read the EXE header");
			std::exit(2);
		}

		// there is not parsing nor validation for the file header, just a bit
		// of logging to ease the debug
		if (hdr.id() != "PS-X EXE") {
			log->warn("header magic string not found! expected=<PS-X EXE> found=<{}>", hdr.id());
		}

		log->trace("exe header PC=0x{:0>8x} GP=0x{:0>8x} SP/FP=(0x{:0>8x} + 0x{:0>8x})",
		           hdr.pc,
		           hdr.gp,
		           hdr.sp_base,
		           hdr.sp_offset);
		log->trace("exe header ZEROFILL=0x{:0>8x}#{}", hdr.memfill_start, hdr.memfill_size);

		log->info("loading {} bytes at address 0x{:>8x}", hdr.exe_size, hdr.load_address);
		log->info("exe from \"{}\"", hdr.marker());
		if (hdr.exe_size % 2048 != 0) {
			log->warn("exe size must be a multiple of 2048, it is {} instead", hdr.exe_size);
		}

		// the load of an exe is divided in three steps:
		//
		// 1 - zero the requested memory
		// 2 - load the exe code at the requested address
		// 3 - load a minimal bios to initialize the register and jump at the
		// begin of the exe
		if (hdr.memfill_start && hdr.memfill_size) {
			auto zero = bus.ram_slice(hdr.memfill_start, hdr.memfill_size);
			std::fill(zero.begin(), zero.end(), 0);
		}

		auto code = bus.ram_slice(hdr.load_address, hdr.exe_size);
		in.seekg(2048);
		if (!in.read(reinterpret_cast<char*>(code.data()), code.size())) {
			log->critical("cannot read the EXE data");
			std::exit(2);
		}

		load_exe_bios(bus, hdr.pc, hdr.gp, hdr.sp_base + hdr.sp_offset);
	}
}