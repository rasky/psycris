#pragma once
#include "../bitmask.hpp"
#include "mmap_device.hpp"
#include <functional>

namespace psycris::hw {
	template <typename D>
	class timer_impl : public mmap_device<timer_impl<D>, 12> {
		using base = mmap_device<timer_impl<D>, 12>;

	  public:
		timer_impl(gsl::span<uint8_t, 12> buffer, std::function<void()> f);

		uint16_t value() const { return read<counter_value>() & 0xffff; }

	  protected:
		using mask = bit_mask<class timer_bits>;

		// reset the counter: 0 when counter == 0xffff, 1 when counter == target
		constexpr mask reset = mask{0x0000'0008};
		constexpr mask irq_on_target = mask{0x0000'0010};
		constexpr mask irq_on_end = mask{0x0000'0020};
		// irq repeat: 0 = one shot; 1 = repeatedly
		constexpr mask irq_repeat = mask{0x0000'0040};
		// irq mode: 0 = pulse; 1 = toggle
		constexpr mask irq_mode = mask{0x0000'0080};
		constexpr mask source = mask{0x0000'0300};
		// increment outcome: bit0 = 0 if IRQ requested
		//                    bit1 = 1 if reached the target value
		//                    bit2 = 1 if reached the 0xffff value
		//
		// XXX: bit1 and bit2 should be reset to zero after reding
		constexpr mask outcome = mask{0x0000'1c00}

		/**
		 * Returns the configured source.
		 *
		 * The returned number ranges from 0 to 3; it's meaning depends on the
		 * timer subclass.
		 */
		uint8_t
		    source() const {
			return source(read<counter_value>());
		}

		void inc(uint16_t v) {
			uint16_t mode = read<counter_mode>() & 0xffff;
			uint16_t target = read<counter_target>() & 0xffff;
			uint16_t curr = value();

			bool target_reached = false;
			bool end_reached = false;

			if (reset.test(mode) && target < curr + v) {
				target_reached = true;
				curr = curr + v - target;
			} else {
				end_reached = curr < curr + v;
				curr = curr + v;
			}
			write<counter_value>(curr);

			bool interrupt = (target_reached && irq_on_target.test(v)) || (end_reached && irq_on_end.test(v));
		}

	  private:
		using counter_value = data_reg<0, 4>;
		using counter_mode = data_reg<4, 4>;
		using counter_target = data_reg<8, 4>;

		friend base;

		void wcb(counter_mode, uint32_t value, uint32_t) { write<counter_value>(0); }

		std::function<void()> make_interrupt;
	};

	class timer0 : public timer_impl<timer0> {
	  public:
		static constexpr char const* device_name = "Timer 0 (SYS + DOT)";

	  public:
		enum source {
			system_clock,
			dot_clock,
		};

		void increment(source, uint32_t q);

		enum event {
			hblank_enter,
			hblank_exit,
		};

		void sync(event);
	};

	class timer1 : public timer_impl<timer1> {
	  public:
		static constexpr char const* device_name = "Timer 1 (SYS + HBLANK)";

	  public:
		enum source { system_clock, hblank };

		void increment(source, uint32_t q);

		enum event {
			vblank_enter,
			vblank_exit,
		};

		void sync(event);
	};

	class timer2 : public timer_impl<timer2> {
	  public:
		static constexpr char const* device_name = "Timer 2 (SYS + SYS/8)";

	  public:
		enum source { system_clock };

		void increment(source, uint32_t q);

		enum event {};

		void sync(event);
	};
}
