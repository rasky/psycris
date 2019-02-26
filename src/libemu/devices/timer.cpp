#include "timer.hpp"
#include "../bitmask.hpp"

namespace {
	using mask = psycris::bit_mask<class timer_bits, uint16_t>;

	static constexpr mask timer_sync_status = mask{0x0000'0001};

	static constexpr mask timer_sync_mode = mask{0x0000'0006};

	// reset the counter: 0 when counter == 0xffff, 1 when counter == target
	static constexpr mask reset = mask{0x0000'0008};

	static constexpr mask irq_on_target = mask{0x0000'0010};
	static constexpr mask irq_on_end = mask{0x0000'0020};

	// irq repeat: 0 = one shot; 1 = repeatedly
	static constexpr mask irq_repeat = mask{0x0000'0040};

	// irq mode: 0 = pulse; 1 = toggle
	static constexpr mask irq_mode = mask{0x0000'0080};
	static constexpr mask timer_source = mask{0x0000'0300};

	static constexpr mask irq_requested = mask{0x0000'0400};
	static constexpr mask target_reached = mask{0x0000'0800};
	static constexpr mask end_reached = mask{0x0000'1000};
}

namespace psycris::hw {

	base_timer::sync_status_t base_timer::sync_status() const {
		uint32_t mode = read<counter_mode>();
		return timer_sync_status(mode).as<sync_status_t>();
	}

	uint8_t base_timer::sync_mode() const {
		uint32_t mode = read<counter_mode>();
		return timer_sync_mode(mode);
	}

	uint8_t base_timer::source() const {
		uint32_t mode = read<counter_mode>();
		return timer_source(mode);
	}

	uint16_t base_timer::value() const { return read<counter_value>() & 0xffff; }

	void base_timer::increment(uint16_t v) {
		uint16_t mode = read<counter_mode>() & 0xffff;
		uint16_t target = read<counter_target>() & 0xffff;
		uint16_t curr = value();

		bool is_target_reached = target_reached(mode);
		bool is_end_reached = end_reached(mode);

		is_target_reached |= target <= curr + v;
		is_end_reached |= curr <= curr + v;

		if (reset.test(mode) && target <= curr + v) {
			curr = curr + v - target;
		} else {
			curr = curr + v;
		}

		write<counter_value>(curr);

		bool interrupt = (is_target_reached && irq_on_target.test(v)) || (is_end_reached && irq_on_end.test(v));

		target_reached(mode) = is_target_reached;
		end_reached(mode) = is_end_reached;
		irq_requested(mode) = interrupt;

		write<counter_mode>(mode);
	}

	void timer0::input(source src, uint32_t v) {
		uint8_t enabled_source = base_timer::source();
		if ((enabled_source & 0x1) == 0 && src == source::system_clock) {
			increment(v);
		} else if ((enabled_source & 0x1) == 1 && src == source::dot_clock) {
			increment(v);
		}
	}
}