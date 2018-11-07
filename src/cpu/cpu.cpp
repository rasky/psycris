#include "cpu.hpp"
#include "disassembly.hpp"
#include <cassert>
#include <cstdlib>
#include <fmt/format.h>

namespace {
	void unimplemented(uint32_t pc, uint64_t clock, cpu::decoder ins) {
		fmt::print(
		    "\033[31m[CPU] unimplemented instruction pc={:0>8x} clock={} opcode={:0>#2x} funct={:0>#2x}\033[0m\n",
		    pc,
		    clock,
		    ins.opcode(),
		    ins.funct());
		std::exit(99);
	}
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
			// but for the cpu the PC of the current instruction is pointing to
			// the next instruction (the delay slot).
			fmt::print("{:0>8x}: {}\n", pc, disassembly(ins, pc));
			// pc now points to the delay slot
			pc = npc;
			// npc to the instruction after the delay slot
			npc += 4;

			// execute
			switch (ins.opcode()) {
			case 0x00: // ALU Access
				switch (ins.funct()) {
				case 0x00: // SLL -- Shift left logical
					rd() = rt() << ins.shamt();
					break;
				case 0x21: // ADDU -- Add Unsigned Word
					rd() = rs() + rt();
					break;
				case 0x2b: // SLTU -- Set On Less Than Unsigned
					rd() = rs() < rt() ? 1 : 0;
					break;
				case 0x25: // OR -- Bitwise or
					rd() = rs() | rt();
					break;
				default:
					unimplemented(pc, clock, ins);
				}
				break;
			case 0x03: // JAL -- Jump And Link
				regs[31] = npc;
				[[fallthrough]];
			case 0x02: // J -- Jump
				npc = (pc & 0xf000'0000) | (ins.target() << 2);
				break;
			case 0x05: // BNE -- Branch On Not Equal
				if (rs() != rt()) {
					npc = npc + (ins.imm() << 2);
				}
				break;
			case 0x08: { // ADDI -- Add Immediate Word
				uint32_t r;
				if (__builtin_add_overflow(rs(), ins.imm(), &r)) {
					fmt::print("integer overflow, missing exception\n");
					assert(0);
				}
				rt() = r;
				break;
			}
			case 0x09: // ADDIU -- Add immediate unsigned (no overflow)
				rt() = rs() + ins.imm();
				break;
			case 0x0c: // ANDI -- And Immediate
				rt() = rs() & ins.uimm();
				break;
			case 0x0d: // ORI -- Bitwise or immediate
				rt() = rs() | ins.uimm();
				break;
			case 0x0f: // LUI -- Load upper immediate
				rt() = ins.uimm() << 16;
				break;
			case 0x23: // LW -- Load word
				rt() = bus->read<uint32_t>(rs() + ins.imm());
				break;
			case 0x28: // SB -- Store Byte
				bus->write(rs() + ins.imm(), static_cast<uint8_t>(rt()));
				break;
			case 0x29: // SH -- Store Halfword
				bus->write(rs() + ins.imm(), static_cast<uint16_t>(rt()));
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
				unimplemented(pc, clock, ins);
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