#pragma once
#include "bus.hpp"
#include "cop0.hpp"
#include "decoder.hpp"
#include <array>
#include <cstdint>

namespace cpu {
	class mips {
	  public:
		static const uint32_t reset_vector = 0x1FC0'0000;
		static const uint32_t noop = 0;

	  public:
		mips(data_bus*);

		void reset();

		void run(uint64_t until);

	  private:
		uint32_t& rs();
		uint32_t& rt();
		uint32_t& rd();

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
		template <typename Coprocessor>
		void run_cop(Coprocessor&);

	  private:
		std::array<uint32_t, 32> regs;
		uint64_t clock;

		data_bus* bus;

	  private:
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

		cpu::cop0 cop0;
	};
}