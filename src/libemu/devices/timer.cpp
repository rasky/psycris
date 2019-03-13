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
	enum irq_repeat_mode { one_shot = 0, repeat = 1 };
	static constexpr mask irq_repeat = mask{0x0000'0040};

	// irq mode: 0 = pulse; 1 = toggle
	enum irq_mode_mode { pulse = 0, toggle = 1 };
	static constexpr mask irq_mode = mask{0x0000'0080};

	static constexpr mask timer_source = mask{0x0000'0300};

	static constexpr mask irq_requested = mask{0x0000'0400};
	static constexpr mask target_reached = mask{0x0000'0800};
	static constexpr mask end_reached = mask{0x0000'1000};

	// the "sync bit" is stored on an unused bit
	static constexpr mask timer_sync_bit = mask{0x0000'8000};
}

namespace psycris::hw {
	base_timer::base_timer(gsl::span<uint8_t, size> buffer, std::function<void()> f)
	    : mmap_device{buffer, counter_mode{}}, make_interrupt{f} {}

	base_timer::sync_status_t base_timer::sync_status() const {
		uint32_t mode = read<counter_mode>();
		return timer_sync_status(mode).as<sync_status_t>();
	}

	void base_timer::sync_status(sync_status_t v) {
		uint16_t mode = read<counter_mode>() & 0xffff;
		timer_sync_status(mode) = v;
		write<counter_mode>(mode);
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

	void base_timer::value(uint16_t v) { write<counter_value>(v); }

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

		bool interrupt = (is_target_reached && irq_on_target.test(mode)) || (is_end_reached && irq_on_end.test(mode));
		// irq_requested is 0 when an irq is already requested (it is reset to 1
		// during a write).
		//
		// When in one_shot mode we must request an interrupt only once.
		if (irq_repeat(mode) == one_shot && interrupt) {
			if (irq_requested.test(mode)) {
				// the one_shot/toggle config does not trigger any interrupt
				if (irq_mode(mode) == pulse) {
					make_interrupt();
				}
				irq_requested.clear(mode);
			}
		} else if (irq_repeat(mode) == repeat) {
			if (irq_mode(mode) == pulse) {
				if (interrupt) {
					make_interrupt();
				}
				irq_requested(mode) = !interrupt;
			} else {
				if (interrupt) {
					irq_requested(mode) = !irq_requested(mode);
					if (irq_requested.test(mode)) {
						make_interrupt();
					}
				}
			}
		}

		target_reached(mode) = is_target_reached;
		end_reached(mode) = is_end_reached;

		write<counter_mode>(mode);
	}

	void base_timer::sync_bit(bool v) {
		uint16_t mode = read<counter_mode>() & 0xffff;
		timer_sync_bit(mode) = v;
		write<counter_mode>(mode);
	}

	bool base_timer::sync_bit() const { return timer_sync_bit(read<counter_mode>()); }

	void base_timer::wcb(counter_mode, uint32_t new_value, uint32_t) {
		uint16_t mode = new_value;
		irq_requested.set(mode);
		write<counter_mode>(mode);
		write<counter_value>(0);
	}

	namespace {
		bool timer01_can_increment(uint8_t source, base_timer const& t) {
			bool do_increment = ((t.source() & 0x1) == 0 && source == 0) || ((t.source() & 0x1) == 1 && source == 1);

			if (!do_increment) {
				return false;
			}

			/* sync_mode
			 * ---------
			 * 0 - Pause during Hblank/VBlank
			 * 1 - Reset counter to 0 at Hblank/VBlank
			 * 2 - Reset counter to 0 and pause outside of Hblank/VBlank
			 * 3 - Pause until Hblank/VBlank occurs once, then switch to free run
			 */
			if (t.sync_status() == base_timer::enabled) {
				uint8_t mode = t.sync_mode();
				bool blank = t.sync_bit();
				if (mode == 3) {
					do_increment = false;
				} else if ((mode == 0 && blank) || (mode == 2 && !blank)) {
					do_increment = false;
				}
			}

			return do_increment;
		}
	}

	void timer0::input(source src, uint32_t v) {
		if (timer01_can_increment(src, *this)) {
			increment(v);
		}
	}

	void timer0::enter_hblank() {
		sync_bit(true);
		if (sync_status() == enabled) {
			uint8_t mode = sync_mode();
			if (mode == 1 || mode == 2) {
				value(0);
			} else if (mode == 3) {
				sync_status(base_timer::disabled);
			}
		}
	}

	void timer0::exit_hblank() { sync_bit(false); }

	void timer1::input(source src, uint32_t v) {
		if (timer01_can_increment(src, *this)) {
			increment(v);
		}
	}

	void timer1::enter_vblank() {
		sync_bit(true);
		if (sync_status() == enabled) {
			uint8_t mode = sync_mode();
			if (mode == 1 || mode == 2) {
				value(0);
			} else if (mode == 3) {
				sync_status(base_timer::disabled);
			}
		}
	}

	void timer1::exit_vblank() { sync_bit(false); }
}