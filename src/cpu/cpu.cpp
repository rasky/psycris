#include "cpu.hpp"
#include "opcodes.hpp"
#include <cassert>

namespace cpu {
	mips::mips(memory_bus* b) : bus{b} {
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
		using opcodes::decoder;
		using opcodes::execute;

		namespace op = opcodes;

		while (clock < until) {
			auto opcode = bus->read<uint32_t>(pc);

			switch (decoder::opcode(opcode)) {
			case 0x00:
				switch (decoder::funct(opcode)) {
				case op::add::funct:
					execute<op::add>(*this, opcode);
					break;
				}
				break;
			case op::addi::opcode:
				execute<op::addi>(*this, opcode);
				break;

			default:
				printf("[CPU] unimplemented opcode: %02x\n", decoder::opcode(opcode));
				assert(0);
			}

			pc += 4;
		}
	}
}