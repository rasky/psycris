#pragma once
#include "../hw/bus.hpp"
#include "cop0.hpp"
#include "decoder.hpp"

#include <array>
#include <cstdint>
#include <iosfwd>

namespace bus = psycris::bus;

namespace psycris::cpu {
	class mips {
	  public:
		static const uint32_t exc_vector = 0x8000'0080;
		static const uint32_t rom_exc_vector = 0xbfc0'0180;
		static const uint32_t reset_vector = 0x1FC0'0000;

		static const uint32_t noop = 0;

	  public:
		mips(bus::data_bus&);

	  public:
		void reset();

		void trap(cop0::exc_code);

		void run(uint64_t until);

	  public:
		uint64_t ticks() const;

	  private:
		uint32_t& rs();
		uint32_t& rt();
		uint32_t& rd();

		uint32_t& hi();
		uint32_t& lo();

	  private:
		/**
		 * \brief Reads from the bus
		 *
		 * An Address error hw exception is raised if the address is not
		 * properly aligned.
		 */
		template <typename T>
		T read(uint32_t) const;

		/**
		 * \brief Writes to the bus
		 *
		 * An Address error hw exception is raised if the address is not
		 * properly aligned.
		 */
		template <typename T>
		void write(uint32_t, T) const;

	  private:
		template <typename B>
		void add_with_overflow(uint32_t& c, uint32_t a, B b);

	  private:
		template <typename Coprocessor>
		void run_cop(Coprocessor&);

	  public:
		std::array<uint32_t, 32> regs;

		// The two registers associated with the integer multipler.
		//
		// These registers, referred to as “HI” and “LO”, contain the 64-bit
		// product result of a multiply operation, or the quotient and remainder
		// of a divide.
		std::array<uint32_t, 2> mult_regs;

		cpu::cop0 cop0;

	  private:
		uint64_t clock;

		bus::data_bus* bus;

		// the current instruction; the one executed during this clock cycle
		decoder ins;
		// the pc of the current instruction
		uint32_t pc;

		// the next instruction; this one is located at `npc` and prefetched
		// during this clock cycle.
		//
		// `next_ins` becomes the current instruction at the end of each clock
		// cycle, this behavior is useful to simulate the `delay slots` of the
		// mips.
		decoder next_ins;
		uint32_t npc;

		friend void dump_cpu(std::ostream&, mips const&);
		friend void restore_cpu(std::istream&, mips&);
	};

	/**
	 * \brief Writes the cpu state into the output stream
	 *
	 * The following data are written using the stream unformatted functions:
	 *
	 * | Offset | Value               | Bytes
	 * | ------ | ------------------- | -----
	 * | 0      | clock               | 8
	 * | 8      | current instruction | 4
	 * | 12     | pc                  | 4
	 * | 16     | next instruction    | 4
	 * | 20     | npc                 | 4
	 * | 24     | regs[0..31]         | 128
	 * | 152    | mult regs (lo/hi)   | 8
	 * | 160    | cop0 regs[0..31]    | 128
	 *
	 */
	void dump_cpu(std::ostream&, mips const&);

	void restore_cpu(std::istream&, mips&);
}