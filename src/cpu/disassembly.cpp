#include "disassembly.hpp"
#include "decoder.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <unordered_map>
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

	template <typename T, typename>
	struct opaque {
		T value;

		opaque(T v) : value{v} {
		}

		bool operator==(T o) const {
			return value == o;
		}
	};

	using reg = opaque<uint8_t, class reg_>;
	using i16 = opaque<uint16_t, class i16_>;
	using i5 = opaque<uint8_t, class i5_>;

	std::ostream& operator<<(std::ostream& os, reg r) {
		if (r.value >= aliases.size()) {
			fmt::print(os, "R??");
		} else {
			fmt::print(os, "{}({})", aliases[r.value], r.value);
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, i16 r) {
		fmt::print(os, "{:#x}", r.value);
		return os;
	}

	std::ostream& operator<<(std::ostream& os, i5 r) {
		fmt::print(os, "{:#x}", r.value);
		return os;
	}

	std::string unknown(uint32_t ins) {
		cpu::decoder d{ins};
		return fmt::format("{:0>8x} [{:0>32b}] opcode={:0>#2x} funct={:0>#2x}", ins, ins, d.opcode(), d.funct());
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

		i5 shamt() const {
			return i5{cpu::decoder::shamt()};
		}

		i16 imm() const {
			// for the disassembler I ignore the sign extension because this is
			// the way the user write the asm
			return i16(ins & 0xffff);
		}
	};
}

namespace {
	// clang-format off
	std::unordered_map<uint8_t, std::string> iu_ins{
        {0x0f, "lui {rt}, {imm16}"},
        {0x09, "addiu {rt}, {rs}, {imm16}"},
		{0x0d, "ori {rt}, {rs}, {imm16}"},
		{0x2b, "sw {rt}, {imm16}({rs})"},
    };

    std::unordered_map<uint8_t, std::string> iu_funct_ins{
		{0x00, "sll {rd}, {rt}, {imm5}"},
		{0x25, "or {rd}, {rs}, {rt}"},
    };
	// clang-format on

	std::string integer_unit(::decoder dec, uint32_t pc) {
		std::string fmt;
		{
			auto r = iu_ins.find(dec.opcode());
			if (r != iu_ins.end()) {
				fmt = r->second;
			}
		}

		if (fmt == "" && dec.opcode() == 0) {
			// the opcode 0 is shared, we need to check the funct value
			if (dec.ins == 0) {
				fmt = "noop";
			} else if (dec.funct() == 0x25 && dec.rs() == 0 && dec.rt() == 0) {
				fmt = "move {rd}, zero";
			} else {
				auto r = iu_funct_ins.find(dec.funct());
				if (r != iu_funct_ins.end()) {
					fmt = r->second;
				}
			}
		}

		if (fmt == "") {
			switch (dec.opcode()) {
			case 0x02:
				return fmt::format("j {:#x}", (pc & 0xf000'0000) | (dec.target() << 2));
				break;
			}
		}

		using namespace fmt::literals;
		return fmt::format(fmt,
		                   "rt"_a = dec.rt(),
		                   "rd"_a = dec.rd(),
		                   "rs"_a = dec.rs(),
		                   "imm5"_a = dec.shamt(),
		                   "imm16"_a = dec.imm());
	}

	std::string coprocessor(::decoder dec) {
		std::string fmt = "";
		if (!dec.is_cop()) {
			return fmt;
		}

		if (!dec.is_cop_fn()) {
			switch (dec.cop_subop()) {
			case 0x00:
				fmt = "mfc{cn} {rt}, {rd}";
				break;
			case 0x04:
				fmt = "mtc{cn} {rt}, {rd}";
				break;
			}
		}

		using namespace fmt::literals;
		return fmt::format(fmt, "rt"_a = dec.rt(), "rd"_a = dec.rd(), "rs"_a = dec.rs(), "cn"_a = dec.cop_n());
	}
}

namespace cpu {
	std::string disassembly(uint32_t ins, uint32_t pc) {
		::decoder dec{ins};

		std::string s = integer_unit(dec, pc);
		if (s == "") {
			s = coprocessor(dec);
		}
		if (s == "") {
			s = unknown(ins);
		}
		return s;
	}
}