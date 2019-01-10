#pragma once
#include "../mmap_device.hpp"

namespace cpu {
	class cop0;
}

namespace psycris::hw {
	class interrupt_control : public mmap_device<interrupt_control, 8> {
	  public:
		static constexpr char const* device_name = "Interrupt Control";

		interrupt_control(gsl::span<uint8_t, size> buffer, cpu::cop0& cop);

	  public:
		enum interrupt {
			VBLANK = 0x0000'0001,
			GPU = 0x0000'0002,
			CDROM = 0x0000'0004,
			DMA = 0x0000'0008,
			TMR0 = 0x0000'0010,
			TMR1 = 0x0000'0020,
			TMR2 = 0x0000'0040,
			MEM_CARD = 0x0000'0080,
			SIO = 0x0000'0100,
			SPU = 0x0000'0200,
			LIGHT_PEN = 0x0000'0400
		};

		void request(interrupt);

	  private:
		using i_stat = data_reg<0>;
		using i_mask = data_reg<4>;

		friend mmap_device;

		void wcb(i_stat, uint32_t, uint32_t);

	  private:
		cpu::cop0* cop0;
	};
}