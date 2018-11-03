#include "cpu.hpp"
#include <cassert>
#include <fmt/format.h>

namespace {
	std::string format(uint32_t ins) {
		cpu::decoder d{ins};
		return fmt::format("{:x} [{:0>32b}] opcode={:0>2x}", ins, ins, d.opcode());
	}
}

namespace cpu {
	mips::mips(data_bus* b) : bus{b} {
		reset();
	}

	void mips::reset() {
		for (int i = 0; i < 32; i++) {
			regs[i] = 0;
		}
		clock = 0;
		pc = mips::reset_vector;
	}

	void mips::run(uint64_t until) {
		while (clock < until) {
			dec.ins = bus->read<uint32_t>(pc);

			switch (dec.opcode()) {
			case 0x00: // ALU Access
				switch (dec.funct()) {
				case 0x00: // SLL -- Shift left logical
					rd() = rt() << dec.imm5();
					break;
				default:
					fmt::print("[CPU] unimplemented instruction: {}\n", format(dec.ins));
					assert(0);
				}
				break;
			case 0x0f: // LUI -- Load upper immediate
				rt() = dec.imm16() << 16;
				break;
			case 0x0d: // ORI -- Bitwise or immediate
				rt() = rs() | dec.imm16();
				break;
			case 0x2b: // SW -- Store word
				bus->write(rs() + dec.imm16(), rt());
				break;
			default:
				fmt::print("[CPU] unimplemented instruction: {}\n", format(dec.ins));
				assert(0);
			}

			pc += 4;
		}
	}

	uint32_t& mips::rs() {
		return regs[dec.rs()];
	}

	uint32_t& mips::rt() {
		return regs[dec.rs()];
	}

	uint32_t& mips::rd() {
		return regs[dec.rs()];
	}
}