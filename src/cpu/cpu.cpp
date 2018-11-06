#include "cpu.hpp"
#include "disassembly.hpp"
#include <cassert>
#include <fmt/format.h>

namespace cpu {
	namespace {}
}

namespace cpu {
	mips::mips(data_bus* b) : bus{b} {
		reset();
	}

	void mips::reset() {
		regs.fill(0);
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
					rd() = rt() << ins.shamt();
					break;
				case 0x25: // OR -- Bitwise or
					rd() = rs() | rt();
					break;
				default:
					fmt::print("[CPU] unimplemented instruction\n");
					assert(0);
				}
				break;
			case 0x02: // J -- Jump
				npc = (pc & 0xf000'0000) | (ins.target() << 2);
				break;
			case 0x05: // BNE -- Branch On Not Equal
				if (rs() != rt()) {
					npc = npc + (ins.imm() << 2);
				}
				break;
			case 0x08: // ADDI -- Add Immediate Word
				rt() = rs() + ins.imm();
				break;
			case 0x09: // ADDIU -- Add immediate unsigned (no overflow)
				rt() = rs() + ins.imm();
				break;
			case 0x0d: // ORI -- Bitwise or immediate
				rt() = rs() | ins.imm();
				break;
			case 0x0f: // LUI -- Load upper immediate
				rt() = ins.imm() << 16;
				break;
			case 0x23:
				rt() = bus->read<uint32_t>(rs() + ins.imm());
				break;
			case 0x2b: // SW -- Store word
				bus->write(rs() + ins.imm(), rt());
				break;
			case 0x10: // COP0
				run_cop(cop0);
				break;
			case 0x12: // COP2
				fmt::print("[CPU] unimplemented instruction for coprocessor 2\n");
				break;
			case 0x11: // COP1
			case 0x13: // COP3
				fmt::print("[CPU] instruction for unavailable coprocessor {}\n", ins.cop_n());
				break;
			default:
				fmt::print("\033[31m[CPU] unimplemented instruction\033[0m\n");
				assert(0);
			}

			ins = next_ins;
		}
	}

	template <typename Coprocessor>
	void mips::run_cop(Coprocessor& cop) {
		if (ins.is_cop_fn()) {
			fmt::print("[CPU][COP] unimplemented 'cop command' {}\n", ins.cop_fn());
			return;
		}
		switch (ins.cop_subop()) {
		case 0x04: // MTC
			cop.regs[ins.rt()] = rd();
			break;
		default:
			fmt::print("[CPU][COP] unimplemented instruction\n");
			assert(0);
		}
	}

	uint32_t& mips::rs() {
		return regs[ins.rs()];
	}

	uint32_t& mips::rt() {
		return regs[ins.rt()];
	}

	uint32_t& mips::rd() {
		return regs[ins.rd()];
	}
}