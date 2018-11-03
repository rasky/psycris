#include "disassembly.hpp"
#include "decoder.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <vector>

namespace {
	// clang-format off
	std::vector<char const*> aliases = {
	    // Constant (always 0) (this one isn't a real register)
	    "zero",
	    // Assembler temporary (destroyed by some pseudo opcodes!)
	    "at",
	    // Subroutine return values, may be changed by subroutines
	    "v0", "v1",
	    // Subroutine arguments, may be changed by subroutines
	    "a0", "a1", "a2", "a3",
	    // Temporaries, may be changed by subroutines
	    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	    // Static variables, must be saved by subs
	    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	    // Temporaries, may be changed by subroutines
	    "t8", "t9",
	    // Reserved for kernel (destroyed by some IRQ handlers!)
	    "k0", "k1",
	    // Global pointer (rarely used)
	    "gp",
	    // Stack pointer
	    "sp",
	    // Frame Pointer, or 9th Static variable, must be saved
	    "fp(s8)",
	    // Return address (used so by JAL,BLTZAL,BGEZAL opcodes)
	    "ra",
	};
	// clang-format on

	struct reg {
		uint8_t value;
	};

	std::ostream& operator<<(std::ostream& os, reg r) {
		if (r.value >= aliases.size()) {
			fmt::print(os, "R??");
		} else {
			fmt::print(os, "{}({})", aliases[r.value], r.value);
		}
		return os;
	}

	std::string unknown(uint32_t ins) {
		cpu::decoder d{ins};
		return fmt::format("{:x} [{:0>32b}] opcode={:0>2x} funct={:0>2x}", ins, ins, d.opcode(), d.funct());
	}

	struct decoder : cpu::decoder {
		reg rs() const {
			return reg{cpu::decoder::rs()};
		}

		reg rt() const {
			return reg{cpu::decoder::rt()};
		}

		reg rd() const {
			return reg{cpu::decoder::rd()};
		}

		uint16_t imm16() const {
			return ins & 0xffff;
		}
	};
}

namespace cpu {
	std::string disassembly(uint32_t ins) {
		::decoder dec{ins};

		switch (dec.opcode()) {
		case 0x00:
			switch (dec.funct()) {
			case 0x00:
				if (ins == 0) {
					return "noop";
				}
				return fmt::format("sll {}, {}, {:#x}", dec.rd(), dec.rt(), dec.imm5());
			}
			break;
		case 0x09:
			return fmt::format("addiu {}, {}, {:#x}", dec.rt(), dec.rs(), dec.imm16());
		case 0x0d:
			return fmt::format("ori {}, {}, {:#x}", dec.rt(), dec.rs(), dec.imm16());
		case 0x0f:
			return fmt::format("lui {}, {:#x}", dec.rt(), dec.imm16());
		case 0x2b:
			return fmt::format("sw {}, {}({})", dec.rt(), dec.imm16(), dec.rs());
		}
		return unknown(ins);
	}
}