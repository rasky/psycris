#pragma once
#include <array>
#include <cstdint>
#include <fmt/format.h>
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
		template <typename T>
		T read(uint32_t addr) {
			switch (addr >> 20) {
			case 0x1FC:
			case 0x9FC:
			case 0xBFC:
				// fmt::print("[BUS][BIOS] address={:x} offset={:x}\n", addr, addr & 0xf'ffff);
				// undefined behaviour?
				return *(reinterpret_cast<T*>(&bios[addr & 0xf'ffff]));
				break;
			default:
				fmt::print("[BUS] unmapped read: {:x}", addr);
				return {};
			}
		}

		template <typename T>
		void write(uint32_t addr, T val) {
			switch (addr >> 24) {
			case 0x00:
			case 0x80:
			case 0xa0:
				fmt::print("[BUS][RAM] write to address={:x}\n", addr);
				*(reinterpret_cast<T*>(&ram[addr & 0xff'ffff])) = val;
				break;
			}
			fmt::print("[BUS] write {:#x} = {:#x}\n", addr, val);
		}

	  private:
		gsl::span<uint8_t> bios;
		gsl::span<uint8_t> ram;
	};

	// clang-format off
	struct decoder {
		uint32_t ins;

		uint8_t opcode() const { return ins >> 26; }

		uint8_t funct() const{ return ins & 0x3f; }

		uint8_t rs() const { return (ins >> 21) & 0x1F; }
 
		uint8_t rt() const { return (ins >> 16) & 0x1F; }
 
 		uint8_t rd() const { return (ins >> 11) & 0x1F; }

		int32_t imm16() const{
			// Note: A const arithmetic immediate values are sign-extended. After
			// that, t consty are handled as signed or unsigned 32 bit numbers,
			// dependi const upon the instruction. The only difference between
			// signed  constd unsigned instructions is that signed instructions can
			// generat constan overflow exception and unsigned instructions can not.
			return static_cast<int32_t>((ins & 0x3ff) << 22) >> 22;
		}

        uint8_t imm5() const {
            return (ins & 0x7ff) >> 6;
        }
	};
	// clang-format on

	class mips {
	  public:
		static const uint32_t reset_vector = 0x1FC0'0000;

	  public:
		std::array<uint32_t, 32> regs;
		uint32_t pc;
		uint64_t clock;

		data_bus* bus;

		constexpr uint32_t& operator[](uint8_t ix) {
			return regs[ix];
		}

	  public:
		mips(data_bus*);

		void reset();

		void run(uint64_t until);

	  private:
		uint32_t& rs();
		uint32_t& rt();
		uint32_t& rd();

		decoder dec;
	};
}