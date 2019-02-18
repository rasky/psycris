#pragma once
#include "mmap_device.hpp"
#include <functional>

namespace psycris::hw {
	template <typename D>
	class timer_impl : public mmap_device<timer_impl<D>, 12> {
		using base = mmap_device<timer_impl<D>, 12>;

	  public:
		timer_impl(gsl::span<uint8_t, 12> buffer, std::function<void()> f);

	  private:
		using counter_value = data_reg<0, 4>;
		using counter_mode = data_reg<4, 4>;
		using counter_target = data_reg<8, 4>;

		friend base;

		void wcb(counter_mode, uint32_t, uint32_t);

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