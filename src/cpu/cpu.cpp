#include "cpu.hpp"
#include "disassembly.hpp"
#include <cassert>
#include <fmt/format.h>

namespace cpu {
	mips::mips(data_bus* b) : bus{b} {
		reset();
	}

	void mips::reset() {
		for (auto& r : regs) {
			r = 0;
		}
		clock = 0;

		ins = mips::noop;
		pc = mips::reset_vector - 4;

		next_ins = mips::noop;
		npc = mips::reset_vector;
	}

	void mips::run(uint64_t until) {
		while (clock++ < until) {
			// prefecth the next instruction
			next_ins = bus->read<uint32_t>(npc);

			// adjust the program counters; for debug we want to print the
			// current instruction along with the location where we fetched it,
			// but for the cpu the PC of the current instruction is such
			// location + 4
			fmt::print("{:0>8x}: {}\n", pc, disassembly(ins, pc));
			pc = npc;
			npc += 4;

			// execute
			switch (ins.opcode()) {
			case 0x00: // ALU Access
				switch (ins.funct()) {
				case 0x00: // SLL -- Shift left logical
					rd() = rt() << ins.imm5();
					break;
				case 0x25: // SLL -- Shift left logical
					rd() = rs() | rt();
					break;
				default:
					fmt::print("[CPU] unimplemented instruction\n");
					assert(0);
				}
				break;
			case 0x02: // J -- Jump
				npc = (pc & 0xf000'0000) | (ins.imm26() << 2);
				break;
			case 0x09: // ADDIU -- Add immediate unsigned (no overflow)
				rt() = rs() + ins.imm16();
				break;
			case 0x0d: // ORI -- Bitwise or immediate
				rt() = rs() | ins.imm16();
				break;
			case 0x0f: // LUI -- Load upper immediate
				rt() = ins.imm16() << 16;
				break;
			case 0x2b: // SW -- Store word
				bus->write(rs() + ins.imm16(), rt());
				break;
			default:
				fmt::print("[CPU] unimplemented instruction\n");
				assert(0);
			}

			ins = next_ins;
		}
	}

	uint32_t& mips::rs() {
		return regs[ins.rs()];
	}

	uint32_t& mips::rt() {
		return regs[ins.rs()];
	}

	uint32_t& mips::rd() {
		return regs[ins.rs()];
	}
}