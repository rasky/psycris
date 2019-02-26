#pragma once
#include "mmap_device.hpp"
#include <functional>

namespace psycris::hw {
	class base_timer : public mmap_device<base_timer, 12> {
	  public:
		base_timer(gsl::span<uint8_t, size> buffer, std::function<void()> f) : mmap_device{buffer}, make_interrupt{f} {}

		enum sync_status_t {
			disabled = 0,
			enabled = 1,
		};

		/**
		 * \brief Returns the synchronization status
		 *
		 * The synchronization status is not used by this class; it is exposed for
		 * the convenience of the subclasses.
		 *
		 * sync_status_t::disabled means no synchronization, the timer is
		 * incremented every time the configured source (see `source()`) is
		 * triggered.
		 *
		 * OTOH sync_status_t::enabled means that the timer is incremented (or
		 * resetted) according to the configured synchronization mode.
		 */
		sync_status_t sync_status() const;

		/**
		 * \brief Returns the synchronization mode
		 *
		 * The synchronization mode is not used by this class; it is exposed for
		 * the convenience of the subclasses.
		 *
		 * The synchronization mode is a number in the range [0-3]; its meaning
		 * depends on the subclass.
		 */
		uint8_t sync_mode() const;

		/**
		 * \brief Returns the timer source
		 *
		 * The timer source it not used by this class; it is exposed for the
		 * convenience of the subclasses.
		 *
		 * The source is a number in the range [0-3]; its meaning
		 * depends on the subclass.
		 */
		// void source(uint8_t v);

		uint8_t source() const;

	  protected:
		/**
		 * \brief Returns the current timer value
		 */
		uint16_t value() const;

		void increment(uint16_t v);

	  private:
		using counter_value = data_reg<0, 4>;
		using counter_mode = data_reg<4, 4>;
		using counter_target = data_reg<8, 4>;

		// 	void wcb(counter_mode, uint32_t, uint32_t) { this->template write<counter_value>(0); }

		std::function<void()> make_interrupt;
	};

	class timer0 : public base_timer {
	  public:
		static constexpr char const* device_name = "Timer 0 (SYS + DOT)";

		using base_timer::base_timer;

	  public:
		enum source {
			system_clock,
			dot_clock,
		};

		void input(source src, uint32_t v);

		using base_timer::value;
	};
	/*
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
	    */
}
