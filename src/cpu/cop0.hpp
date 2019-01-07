#pragma once
#include "../bitmask.hpp"

#include <array>

namespace cpu {
	namespace cause_bits {
		using mask = psycris::bit_mask<class cause_bits_>;

		// "branch delay": if set, this bit indicates that the EPC does not
		// point to the actual “exception” instruction, but rather to the
		// branch instruction which immediately precedes it.
		// When the exception restart point is an instruction which is in the
		// "delay slot" following a branch, EPC has to point to the branch
		// instruction; it is harmless to re-execute the branch, but if the
		// CPU returned from the exception to the branch delay instruction
		// itself the branch would not be taken and the exception would
		// have broken the interrupted program.
		// The only time software might be sensitive to this bit is if it must
		// analyze the "offending" instruction (if BD == 1 then the
		// instruction is at EPC + 4). This would occur if the instruction
		// needs to be emulated (e.g. a floating point instruction in a device
		// with no hardware FPA; or a breakpoint placed in a branch delay
		// slot).
		constexpr mask BD{0x8000'0000};

		// "co-processor error": if the exception is taken because a "co-
		// processor" format instruction was for a "co-processor" which is
		// not enabled by the CUx bit in SR, then this field has the co-
		// processor number from that instruction.
		constexpr mask CE{0x3000'0000};

		// "Interrupt Pending": shows the interrupts which are currently
		// asserted (but may be “masked” from actually signalling an
		// exception). These bits follow the CPU inputs for the six hardware
		// levels. Bits 9 and 8 are read/writable, and contain the value last
		// written to them. However, any of the 8 bits active when enabled
		// by the appropriate IM bit and the global interrupt enable flag IEc
		// in SR, will cause an interrupt.
		// IP is subtly different from the rest of the Cause register fields; it
		// doesn’t indicate what happened when the exception took place,
		// but rather shows what is happening now.
		constexpr mask IP{0x0000'ff00};

		// A 5-bit code which indicates what kind of exception happened
		constexpr mask ExcCode{0x0000'007c};
	}

	namespace sr_bits {
		using mask = psycris::bit_mask<class sr_bits_>;
		// IEc is set 0 to prevent the CPU taking any interrupt, 1 to
		// enable.
		constexpr mask IEc{0x0000'0001};

		// KUc is set 1 when running with kernel privileges, 0 for user
		// mode.
		constexpr mask KUc{0x0000'0002};

		// on an exception, the hardware takes the values of KUc and IEc and
		// saves them here; at the same time as changing the values of KUc,
		// IEc to [1, 0] (kernel mode, interrupts disabled). The instruction
		// rfe can be used to copy KUp, IEp back into KUc, IEc.
		constexpr mask IEp{0x0000'0004};
		constexpr mask KUp{0x0000'0008};

		// on an exception the KUp, IEp bits are saved here. Effectively,
		// the six KU/IE bits are operated as a 3-deep, 2-bit wide stack
		// which is pushed on an exception and popped by an rfe .
		constexpr mask IEo{0x0000'0010};
		constexpr mask KUo{0x0000'0020};

		// an handy mask that covers all the exception stacks
		constexpr mask AllExcBits{0x0000'003f};

		// isolate (data) cache. IsC set 1: makes all loads and stores
		// access only the data cache, and never memory
		constexpr mask IsC{0x0001'0000};

		// "boot exception vectors": when BEV == 1, the CPU uses the ROM (kseg1)
		// space exception entry point . BEV is usually set to zero in running
		// systems; this relocates the exception vectors. to RAM addresses,
		// speeding accesses and allowing the use of "user supplied" exception
		// service routines.
		constexpr mask BEV{0x0040'0000};
	}

	class cop0 {
	  public:
		cop0() = default;

	  public:
		enum registers {
			// CP0 type and rev level
			PRId = 15,
			//(status register) CPU mode flags
			SR = 12,
			// Describes the most recently recognized exception
			Cause = 13,
			// Return address from trap
			// This is a 32-bit register containing the 32-bit address of the
			// return point for this exception. The instruction causing (or
			// suffering) the exception is at EPC, unless BD is set in Cause, in
			// which case EPC points to the previous (branch) instruction.
			EPC = 14,
			//  Contains the last invalid program address which caused a trap.
			//  It is set by address errors of all kinds, even if there is no
			//  MMU
			// A 32-bit register containing the address whose reference led to
			// an exception; set on any MMU-related exception, on an attempt by
			// a user program to access addresses outside kuseg, or if an
			// address is wrongly aligned for the datum size referenced. After
			// any other exception this register is undefined. Note in
			// particular that it is not set after a bus error.
			BadVaddr = 8,
		};

	  public:
		void reset();

		void interrupt_request();

	  public:
		enum exc_code {
			Int = 0, // Interrupt
			Mod,     // TLB modification
			TLBL,    // TLB load
			TLBS,    // TLB store
			// Address error (on load/I-fetch or store respectively). Either an
			// attempt to access outside kuseg when in user mode, or an attempt
			// to read a word or half-word at a misaligned address.
			AdEL,
			AdES,
			// Bus error (instruction fetch or data load, respectively).
			// External hardware has signalled an error of some kind;
			// proper exception handling is system-dependent. The
			// R30xx family CPUs can’t take a bus error on a store;
			// the write buffer would make such an exception
			// “imprecise”.
			IBE,
			DBE,
			Syscall, // Generated unconditionally by a syscall instruction.
			Bp,      // Breakpoint - a break instruction.
			Ri,      // "reserved instruction"
			CpU,     // "Co-Processor unusable"
			Ov,      // "arithmetic overflow". Note that ‘‘unsigned’’ versions of
			         // instructions (e.g. addu ) never cause this exception.
		};

		void restore_from_exception() { sr_bits::AllExcBits(sr()) >>= 2; }

		void enter_exception(exc_code c) {
			// save the current values of KU and IE
			sr_bits::AllExcBits(sr()) <<= 2;
			// change to kernel mode and disable the interrupts
			sr_bits::KUc(sr()) = 1;
			sr_bits::IEc(sr()) = 0;
			// store the exception cause
			cause_bits::ExcCode(cause()) = c;
		}

	  public:
		std::array<uint32_t, 32> regs;

		uint32_t prid() const { return regs[PRId]; }
		uint32_t& prid() { return regs[PRId]; }

		uint32_t sr() const { return regs[SR]; }
		uint32_t& sr() { return regs[SR]; }

		uint32_t cause() const { return regs[Cause]; }
		uint32_t& cause() { return regs[Cause]; }

		uint32_t epc() const { return regs[EPC]; }
		uint32_t& epc() { return regs[EPC]; }

		uint32_t bad_vaddr() const { return regs[BadVaddr]; }
		uint32_t& bad_vaddr() { return regs[BadVaddr]; }
	};
}