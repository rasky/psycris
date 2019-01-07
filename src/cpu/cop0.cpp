#include "cop0.hpp"
#include "../logging.hpp"

namespace cpu {
	void cop0::reset() { regs.fill(0); }

	void cop0::interrupt_request() { psycris::log->critical("interrupt request!"); }
}