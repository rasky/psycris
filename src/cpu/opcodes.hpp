#pragma once
#include "cpu.hpp"

#include <cstdint>
#include <functional>
#include <tuple>

namespace cpu::opcodes {

	struct decoder {
		enum flags {
			rs,
			rt,
			rd,
			shift,
			imm16,
		};

		constexpr static uint8_t opcode(uint32_t opcode) {
			return opcode >> 26;
		}

		constexpr static auto funct(uint32_t opcode) {
			return opcode & 0x3f;
		}

		template <flags F>
		constexpr static auto extract(uint32_t opcode) {
			if constexpr (F == rs) {
				return (opcode >> 21) & 0x1F;
			} else if constexpr (F == rt) {
				return (opcode >> 16) & 0x1F;
			} else if constexpr (F == rd) {
				return (opcode >> 11) & 0x1F;
			} else if constexpr (F == shift) {
				return (opcode >> 6) & 0x1F;
			} else if constexpr (F == imm16) {
				// sign extend
				return static_cast<int32_t>((opcode & 0x3ff) << 22) >> 22;
			}
		}
	};

	/** \brief An R Instruction
	 *
	 * According to https://en.wikibooks.org/wiki/MIPS_Assembly/Instruction_Formats
	 * R Instructions are used when all the data values used by the instruction
	 * are located in registers.
	 */
	template <uint8_t Op, uint8_t Funct, decoder::flags... Regs>
	struct r_ins_ {
		constexpr static uint8_t opcode = Op;
		constexpr static uint8_t funct = Funct;
		constexpr static auto flags = std::make_tuple(Regs...);
	};

	template <uint8_t Funct, decoder::flags... Regs>
	using r_ins0 = r_ins_<0x0, Funct, Regs...>;

	template <uint8_t Op, decoder::flags... Regs>
	using r_ins = r_ins_<Op, 0x0, Regs...>;

	namespace details {
		template <typename Ins, size_t... F>
		constexpr void decode_and_execute(mips& cpu, uint32_t dw, std::index_sequence<F...>) {
			std::invoke(Ins::execute, cpu, decoder::extract<std::get<F>(Ins::flags)>(dw)...);
		}
	}

	template <typename Ins>
	constexpr void execute(mips& cpu, uint32_t dw) {
		using FlagsIndices = std::make_index_sequence<std::tuple_size_v<decltype(Ins::flags)>>;
		details::decode_and_execute<Ins>(cpu, dw, FlagsIndices{});
	}

	/**
	 * \brief ADD – Add (with overflow)
	 */
	struct add : r_ins0<0x20, decoder::rs, decoder::rt, decoder::rd> {
		constexpr static void execute(mips& cpu, uint8_t rs, uint8_t rt, uint8_t rd) {
			cpu[rd] = static_cast<int32_t>(cpu[rs]) + static_cast<int32_t>(cpu[rt]);
		}
	};

	/**
	 * \brief ADDI -- Add immediate (with overflow)
	 */
	struct addi : r_ins<0x08, decoder::rs, decoder::rt, decoder::imm16> {
		constexpr static void execute(mips& cpu, uint8_t rs, uint8_t rt, int32_t imm) {
			cpu[rt] = static_cast<int32_t>(cpu[rs]) + imm;
		}
	};

	/**
	 * \brief ADDIU -- Add immediate unsigned (no overflow)
	 */
	struct addiu : r_ins<0x09, decoder::rs, decoder::rt, decoder::imm16> {
		constexpr static void execute(mips& cpu, uint8_t rs, uint8_t rt, uint32_t imm) {
			cpu[rt] = cpu[rs] + imm;
		}
	};

	/**
	 * \brief ADDU -- Add unsigned (no overflow)
	 */
	struct addu : r_ins0<0x21, decoder::rs, decoder::rt, decoder::rd> {
		constexpr static void execute(mips& cpu, uint8_t rs, uint8_t rt, uint8_t rd) {
			cpu[rd] = cpu[rs] + cpu[rd];
		}
	};

	/**
	 * \brief AND -- Bitwise and
	 */
	struct and_ : r_ins0<0x27, decoder::rs, decoder::rt, decoder::rd> {
		constexpr static void execute(mips& cpu, uint8_t rs, uint8_t rt, uint8_t rd) {
			cpu[rd] = cpu[rs] & cpu[rd];
		}
	};
}