#pragma once
#include <array>

namespace cpu {
	class cop0 {
	  public:
		cop0() = default;

		void reset() {
			regs.fill(0);
		}

	  public:
		enum sr_bits {
			// IEc is set 0 to prevent the CPU taking any interrupt, 1 to
			// enable.
			IEc = 0x0000'0000,
			// KUc is set 1 when running with kernel privileges, 0 for user
			// mode.
			KUc = 0x0000'0002,
			// on an exception, the hardware takes the values of KUc and IEc and
			// saves them here; at the same time as changing the values of KUc,
			// IEc to [1, 0] (kernel mode, interrupts disabled). The instruction
			// rfe can be used to copy KUp, IEp back into KUc, IEc.
			IEp = 0x0000'0004,
			KUp = 0x0000'0008,
			// on an exception the KUp, IEp bits are saved here. Effectively,
			// the six KU/IE bits are operated as a 3-deep, 2-bit wide stack
			// which is pushed on an exception and popped by an rfe .
			IEo = 0x0000'0010,
			KUo = 0x0000'0020,
			// isolate (data) cache. IsC set 1: makes all loads and stores
			// access only the data cache, and never memory
			IsC = 0x0001'0000,
		};

		uint32_t& sr() {
			return regs[12];
		}

		uint32_t sr() const {
			return regs[12];
		}

	  public:
		void interrupt_request();

	  public:
		std::array<uint32_t, 32> regs;
	};
}