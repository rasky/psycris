#include "dma.hpp"
#include "interrupt_control.hpp"

namespace psycris::hw {
	dma::dma(gsl::span<uint8_t, size> buffer, interrupt_control& icontrol) : mmap_device{buffer}, ic{&icontrol} {
		write<dicr>(0x0765'4321);
	}

	void dma::wcb(dicr, uint32_t new_value, uint32_t old_value) {
		using namespace dicr_bits;

		// acknowledge the flagged channels
		//
		// To ack a flagged channel (ie set the bit to 0) a write of "1" is
		// needed.
		uint32_t ack = flagged_channels(new_value);
		flagged_channels(new_value) &= ~ack;

		// when request is 1 an interrupt can be requested
		uint32_t request = force_irq(new_value)
		    | (master_enable(new_value) & (enabled_channels(new_value) & flagged_channels(new_value)));

		// the master_flag cannot be written, so I can change it here
		master_flag(new_value) = request;

		write<dicr>(new_value);

		if (request && master_flag(old_value)) {
			ic->request(interrupt_control::DMA);
		}
	}
}