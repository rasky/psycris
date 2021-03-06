#pragma once
#include "cpu/cpu.hpp"

#include "hw/bus.hpp"
#include "hw/devices/dma.hpp"
#include "hw/devices/interrupt_control.hpp"
#include "hw/devices/ram.hpp"
#include "hw/devices/spu.hpp"

#include "meta.hpp"
#include <iosfwd>
#include <vector>

namespace psycris {

	class psx {
	  public:
		struct board {
			/**
			 * \brief The board revision used as the verison of the dump files
			 */
			constexpr static uint16_t rev = 0x1;

			using layout = std::tuple<hw::ram, hw::rom, hw::interrupt_control, hw::dma, hw::spu>;

			constexpr static size_t memory_size() {
				return boost::hana::fold_left(to_type_t<layout>, 0, [](int state, auto p) {
					using T = typename decltype(p)::type;
					return state + T::size;
				});
			}
		};

	  public:
		psx();

	  private:
		std::vector<uint8_t> _board_memory;

	  private:
		bus::data_bus _bus;

	  public:
		cpu::mips cpu;

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
		hw::ram ram;
		hw::rom rom;
		hw::interrupt_control interrupt_control;
		hw::dma dma;
		hw::spu spu;

		friend void dump_board(std::ostream&, psx const&);
		friend void restore_board(std::istream&, psx&);
	};

	void dump_board(std::ostream&, psx const&);
	void restore_board(std::istream&, psx&);
}