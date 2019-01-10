#pragma once
#include "../../bitmask.hpp"
#include "../mmap_device.hpp"

namespace psycris::hw {
	class interrupt_control;

	namespace dicr_bits {
		using mask = psycris::bit_mask<class dicr_bits_>;

		// When set an interrupt is emitted regardless every other condition.
		constexpr mask force_irq{0x0000'8000};

		// The flags to enable the dma interrupts, 7 bits for 7 channels.
		constexpr mask enabled_channels{0x003f'0000};

		// The master control switch to enable/disable the interrupts.
		constexpr mask master_enable{0x0080'0000};

		// These 7 bits are set upon DMA completition
		//
		// The flags are set only if the correspondenting bits in
		// `enabled_channels` are set.
		//
		// These bits are acknowledged (reset to zero) by writing a "1".
		constexpr mask flagged_channels{0x3f00'0000};

		/**
		 * \brief Bit31 is a simple readonly flag set to 1 if an interrupt is
		 * requested.
		 *
		 * The rule for the `master_flag` is:
		 * IF b15=1 OR (b23=1 AND (b16-22 AND b24-30)>0) THEN b31=1 ELSE b31=0
		 *
		 * Upon 0-to-1 transition of Bit31, the IRQ3 flag of the
		 * interrupt_control gets set.
		 */
		constexpr mask master_flag{0x8000'0000};
	}

	class dma : public mmap_device<dma, 8> {
	  public:
		static constexpr char const* device_name = "DMA";

		dma(gsl::span<uint8_t, size> buffer, interrupt_control& icontrol);

	  private:
		using dpcr = data_reg<0>;
		using dicr = data_reg<4>;

		friend mmap_device;

		void wcb(dicr, uint32_t, uint32_t);

	  private:
		interrupt_control* ic;
	};
}