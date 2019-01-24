#include "cpu.hpp"
#include "../logging.hpp"
#include "disassembly.hpp"

#include <cassert>
#include <cstdlib>

namespace {
	using psycris::cpu::decoder;

	void unimplemented(uint32_t pc, uint64_t clock, decoder ins) {
		psycris::log->critical("[CPU] unimplemented instruction pc={:0>8x} clock={} opcode={:0>#2x} funct={:0>#2x}",
		                       pc,
		                       clock,
		                       ins.opcode(),
		                       ins.funct());
		std::exit(99);
	}

	// clang-format off
	template <size_t AccessSize> struct bus_align;
	template <> struct bus_align<1> { static constexpr uint8_t mask = 0x0; };
	template <> struct bus_align<2> { static constexpr uint8_t mask = 0x1; };
	template <> struct bus_align<4> { static constexpr uint8_t mask = 0x3; };
	// clang-format on
}

namespace psycris::cpu {
	mips::mips(bus::data_bus& b) : bus{&b} { reset(); }

	void mips::reset() {
		regs.fill(0);
		clock = 0;

		ins = mips::noop;
		pc = mips::reset_vector - 4;

		next_ins = mips::noop;
		npc = mips::reset_vector;
	}

	void mips::trap(cop0::exc_code cause) {
		cop0.epc() = pc;
		cop0.enter_exception(cause);
		if (sr_bits::BEV(cop0.sr()) == 0) {
			npc = mips::exc_vector;
		} else {
			npc = mips::rom_exc_vector;
		}
	}

	void mips::run(uint64_t until) {
		using psycris::log;

		// reg_tracer rtracer;

		while (clock++ < until) {
			// prefecth the next instruction
			next_ins = bus->read<uint32_t>(npc);

			// adjust the program counters; for debug we want to print the
			// current instruction along with the location where we fetched it,
			// but for the cpu the PC of the current instruction is pointing to
			// the next instruction (the delay slot).

			// log->trace("{:0>8x}@{}: {}", pc, clock, disassembly(ins, pc));
			// pc now points to the delay slot
			pc = npc;
			// npc to the instruction after the delay slot
			npc += 4;

			// execute
			switch (ins.opcode()) {
			case 0x00: // ALU Access
				switch (ins.funct()) {
				case 0x00: // SLL -- Shift Word Left logical
					rd() = rt() << ins.shamt();
					break;
				case 0x02: // SRL -- Shift Word Right Logical
					rd() = rt() >> ins.shamt();
					break;
				case 0x03: // SRA -- Shift Word Right Arithmetic
					rd() = static_cast<int32_t>(rt() >> ins.shamt());
					break;
				case 0x04: // SLLV -- Shift Word Left Logical Variable
					rd() = rt() << (rs() & 0x1f);
					break;
				case 0x07: // SRAV -- Shift Word Right Arithmetic Variable
					rd() = static_cast<int32_t>(rt() >> (rs() & 0x1f));
					break;
				case 0x08: // JR -- Jump Register
					npc = rs();
					break;
				case 0x09: // JALR -- Jump And Link Register
					rd() = npc;
					npc = rs();
					break;
				case 0x0c: // SYSCALL -- System Call
					trap(cop0::Syscall);
					break;
				case 0x10: // MFHI -- Move From Hi
					rd() = hi();
					break;
				case 0x11: // MTHI -- Move To Hi
					hi() = rs();
					break;
				case 0x12: // MFLO -- Move From Lo
					rd() = lo();
					break;
				case 0x13: // MTLO -- Move To Lo
					lo() = rs();
					break;
				case 0x18: { // MULT -- Multiply Word
					int64_t r = static_cast<int32_t>(rs()) * static_cast<int32_t>(rt());
					lo() = r & 0x0000'0000'ffff'ffff;
					hi() = r & 0xffff'ffff'0000'0000;
					break;
				}
				case 0x19: { // MULTU -- Multiply Unsigned Word
					uint64_t r = rs() * rt();
					lo() = r & 0x0000'0000'ffff'ffff;
					hi() = r & 0xffff'ffff'0000'0000;
					break;
				}
				case 0x1a: { // DIV -- Divide Word
					auto r = std::div(static_cast<int32_t>(rs()), static_cast<int32_t>(rt()));
					lo() = r.quot;
					hi() = r.rem;
					break;
				}
				case 0x1b: // DIVU -- Divide Unsigned Word
					lo() = rs() / rt();
					hi() = rs() % rt();
					break;
				case 0x20: // ADD -- Add Word
					add_with_overflow(rd(), rs(), rt());
					break;
				case 0x21: // ADDU -- Add Unsigned Word
					rd() = rs() + rt();
					break;
				case 0x23: // SUBU -- Subtract Unsigned Word
					rd() = rs() - rt();
					break;
				case 0x24: // AND -- And
					rd() = rs() & rt();
					break;
				case 0x27: // NOR -- Nor
					rd() = ~(rs() | rt());
					break;
				case 0x2a: // SLT -- Set On Less Than
					rd() = static_cast<int32_t>(rs()) < static_cast<int32_t>(rt()) ? 1 : 0;
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
			case 0x01:
				switch (ins.rt()) {
				case 0x01: // BGEZ -- Branch On Greater Than Or Equal To Zero
					if (static_cast<int32_t>(rs()) >= 0) {
						npc = pc + sx(ins.uimm() << 2, 16);
					}
					break;
				case 0x11: { // BGEZAL -- Branch On Greater Than Or Equal To Zero And Link
					bool c = static_cast<int32_t>(rs()) >= 0;
					regs[31] = npc;
					if (c) {
						npc = pc + sx(ins.uimm() << 2, 16);
					}
					break;
				}
				case 0x00: // BLTZ -- Branch On Less Than Zero
					if (static_cast<int32_t>(rs()) < 0) {
						npc = pc + sx(ins.uimm() << 2, 16);
					}
					break;
				case 0x10: { // BLTZAL -- Branch On Less Than Zero And Link
					bool c = static_cast<int32_t>(rs()) < 0;
					regs[31] = npc;
					if (c) {
						npc = pc + sx(ins.uimm() << 2, 16);
					}
					break;
				}
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
			case 0x04: // BEQ -- Branch On Equal
				if (rs() == rt()) {
					npc = pc + sx(ins.uimm() << 2, 16);
				}
				break;
			case 0x05: // BNE -- Branch On Not Equal
				if (rs() != rt()) {
					npc = pc + sx(ins.uimm() << 2, 16);
				}
				break;
			case 0x06: // BLEZ -- Branch On Less Than Or Equal To Zero
				if (static_cast<int32_t>(rs()) <= 0) {
					npc = pc + sx(ins.uimm() << 2, 16);
				}
				break;
			case 0x07: // BGTZ -- Branch On Greater Than Zero
				if (static_cast<int32_t>(rs()) > 0) {
					npc = pc + sx(ins.uimm() << 2, 16);
				}
				break;
			case 0x08: // ADDI -- Add Immediate Word
				add_with_overflow(rt(), rs(), ins.imm());
				break;
			case 0x09: // ADDIU -- Add immediate unsigned (no overflow)
				rt() = rs() + ins.imm();
				break;
			case 0x0a: // SLTI -- Set On Less Than Immediate
				rt() = static_cast<int32_t>(rs()) < ins.imm() ? 1 : 0;
				break;
			case 0x0b: // SLTIU -- Set On Less Than Immediate Unsigned
				rt() = rs() < static_cast<uint32_t>(ins.imm()) ? 1 : 0;
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
			case 0x20: // LB -- Load byte
				rt() = sx(read<uint8_t>(rs() + ins.imm()), 8);
				break;
			case 0x21: // LH -- Load Halfword
				rt() = sx(read<uint16_t>(rs() + ins.imm()), 16);
				break;
			case 0x23: // LW -- Load word
				rt() = read<uint32_t>(rs() + ins.imm());
				break;
			case 0x24: // LBU -- Load Byte Unsigned
				rt() = read<uint8_t>(rs() + ins.imm());
				break;
			case 0x25: // LHU -- Load Halfword Unsigned
				rt() = read<uint16_t>(rs() + ins.imm());
				break;
			case 0x28: // SB -- Store Byte
				write(rs() + ins.imm(), static_cast<uint8_t>(rt()));
				break;
			case 0x29: // SH -- Store Halfword
				write(rs() + ins.imm(), static_cast<uint16_t>(rt()));
				break;
			case 0x2b: // SW -- Store word
				write(rs() + ins.imm(), rt());
				break;
			case 0x10: // COP0
				run_cop(cop0);
				break;
			case 0x12: // COP2
				log->warn("[CPU] unimplemented instruction for coprocessor 2");
				break;
			case 0x11: // COP1
			case 0x13: // COP3
				log->warn("[CPU] instruction for unavailable coprocessor {}", ins.cop_n());
				break;
			default:
				unimplemented(pc, clock, ins);
			}

			// rtracer.trace(regs);

			ins = next_ins;
		}
	}

	template <typename Coprocessor>
	void mips::run_cop(Coprocessor& cop) {
		using psycris::log;

		if (ins.is_cop_fn()) {
			switch (ins.cop_fn()) {
			case 0x10: { // RFE -- Restore from Exception
				uint32_t& sr = cop.sr();
				sr |= (sr & 0x3c) >> 2;
				break;
			}
			default:
				log->critical("[CPU][COP] unimplemented 'cop command' {}", ins.cop_fn());
				assert(0);
			}
			return;
		}

		switch (ins.cop_subop()) {
		case 0x00: // MFC
			rt() = cop.regs[ins.rd()];
			break;
		case 0x04: // MTC
			log->info("[CPU][COP] PC={:0>8x}@{} reg{} = 0x{:0>8x}", pc - 4, clock, ins.rd(), rt());
			cop.regs[ins.rd()] = rt();
			break;
		default:
			log->warn("[CPU][COP] unimplemented instruction");
			assert(0);
		}
	}

	uint64_t mips::ticks() const { return clock; }

	template <typename T>
	T mips::read(uint32_t addr) const {
		constexpr uint8_t mask = bus_align<sizeof(T)>::mask;
		if ((addr & mask) != 0) {
			psycris::log->error("unaligned read at 0x{:0>8x} (TODO raise hw exception)", addr);
			addr &= ~mask;
		}
		return bus->read<T>(addr);
	}

	template <typename T>
	void mips::write(uint32_t addr, T val) const {
		constexpr uint8_t mask = bus_align<sizeof(T)>::mask;
		using psycris::log;

		if ((addr & mask) != 0) {
			log->error("unaligned write at 0x{:0>8x} (TODO raise hw exception)", addr);
			addr &= ~mask;
		}

		if (sr_bits::IsC(cop0.sr()) != 0) {
			// the cache is "isolated", but I don't want to implement the exact
			// behavior, for now just ignore the request.
			if (val != 0) {
				log->warn("writing a non zero value on an isolated cache. addr={:0>8x} val={:0>8x}", addr, val);
			}
			return;
		}

		bus->write(addr, val);
	}

	uint32_t& mips::rs() { return regs[ins.rs()]; }

	uint32_t& mips::rt() { return regs[ins.rt()]; }

	uint32_t& mips::rd() { return regs[ins.rd()]; }

	uint32_t& mips::hi() { return mult_regs[1]; }

	uint32_t& mips::lo() { return mult_regs[0]; }

	template <typename B>
	void mips::add_with_overflow(uint32_t& c, uint32_t a, B b) {
		uint32_t r;
		if (__builtin_add_overflow(a, b, &r)) {
			psycris::log->critical("integer overflow, TODO raise hw exception");
			std::exit(99);
			// return std::nullopt;
		}
		c = r;
	}

	void dump_cpu(std::ostream& f, mips const& cpu) {
		auto w = [&](auto& v) { f.write(reinterpret_cast<char const*>(&v), sizeof(v)); };

		w(cpu.clock);
		w(cpu.ins.ins);
		w(cpu.pc);
		w(cpu.next_ins.ins);
		w(cpu.npc);

		w(cpu.regs);
		w(cpu.mult_regs);
		w(cpu.cop0.regs);
	}

	void restore_cpu(std::istream& f, mips& cpu) {
		auto r = [&](auto& v) { f.read(reinterpret_cast<char*>(&v), sizeof(v)); };

		r(cpu.clock);
		r(cpu.ins.ins);
		r(cpu.pc);
		r(cpu.next_ins.ins);
		r(cpu.npc);

		r(cpu.regs);
		r(cpu.mult_regs);
		r(cpu.cop0.regs);

		cpu.clock--;
	}
}