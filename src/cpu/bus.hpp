#pragma once
#include "../logging.hpp"
#include "decoder.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <gsl/span>

namespace cpu {
	/**
	 * \brief CPU data bus
	 *
	 *
	 * https://problemkaputt.de/psx-spx.htm#memorymap
	 *
	 * Memory Map
	 *
	 *   KUSEG     KSEG0     KSEG1
	 *   00000000h 80000000h A0000000h  2048K  Main RAM (first 64K reserved for BIOS)
	 *   1F000000h 9F000000h BF000000h  8192K  Expansion Region 1 (ROM/RAM)
	 *   1F800000h 9F800000h    --      1K     Scratchpad (D-Cache used as Fast RAM)
	 *   1F801000h 9F801000h BF801000h  8K     I/O Ports
	 *   1F802000h 9F802000h BF802000h  8K     Expansion Region 2 (I/O Ports)
	 *   1FA00000h 9FA00000h BFA00000h  2048K  Expansion Region 3 (whatever purpose)
	 *   1FC00000h 9FC00000h BFC00000h  512K   BIOS ROM (Kernel) (4096K max)
	 *         FFFE0000h (KSEG2)        0.5K   I/O Ports (Cache Control)
	 *
	 */
	class data_bus {
	  public:
		data_bus(gsl::span<uint8_t> bios, gsl::span<uint8_t> ram) : bios{bios}, ram{ram} {
		}

	  public:
		static const uint32_t open_bus = 0xffff'ffff;

	  public:
		template <typename T>
		T read(uint32_t addr) {
			using psycris::log;

			uint8_t hw = addr >> 24;
			if (hw < 0x1f || (hw >= 0x80 && hw < 0x9f) || (hw >= 0xa0 && hw < 0xbf)) {
				// RAM
				return *(reinterpret_cast<T*>(&ram[addr & 0xff'ffff]));
			}

			switch (addr >> 20) {
			case 0x1FC:
			case 0x9FC:
			case 0xBFC:
				// BIOS
				// undefined behaviour?
				return *(reinterpret_cast<T*>(&bios[addr & 0xf'ffff]));
				break;
			}

			log->warn("[BUS] unmapped read at {:0>8x}", addr);
			return reinterpret_cast<T>(open_bus);
		}

		template <typename T>
		void write(uint32_t addr, T val) {
			using psycris::log;

			switch (addr >> 24) {
			case 0x00:
			case 0x80:
			case 0xa0:
				log->trace("RAM:{:0>8x} << {:#x}", addr, val);
				*(reinterpret_cast<T*>(&ram[addr & 0xff'ffff])) = val;
				break;
			default:
				log->warn("[BUS] unmapped write at {:0>8x}", addr);
			}
		}

	  private:
		gsl::span<uint8_t> bios;
		gsl::span<uint8_t> ram;
	};
}