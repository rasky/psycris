#include <catch2/catch.hpp>

#include "../bitmask.hpp"
#include "../bus/bus.hpp"
#include "../devices/timer.hpp"

using namespace psycris::hw;

namespace {
	enum reset_mode {
		end_reached = 0,
		target_reached = 1,
	};

	class test_timer {
	  public:
		test_timer()
		    : memory{},
		      timer{gsl::span{reinterpret_cast<uint8_t*>(memory.data()), 12}, [this]() { this->interrupts += 1; }},
		      interrupts{0} {
			bus.connect(0, timer);

			select_input(timer0::system_clock);
			target(0);
			reset_mode(end_reached);
		}

		std::array<uint32_t, 3> memory;

		timer0 timer;

		int interrupts;

		using mask = psycris::bit_mask<class mask_>;

		uint16_t value() const { return timer.value(); }

		void target(uint16_t v) { bus.write(8, static_cast<uint32_t>(v)); }

		void reset_mode(reset_mode v) {
			uint32_t data = memory[1];
			mask{0x0000'0008}(data) = static_cast<uint8_t>(v);
			bus.write(4, data);
		}

		void select_input(timer0::source src) {
			uint32_t data = memory[1];
			mask{0x0000'0300}(data) = static_cast<uint8_t>(src);
			bus.write(4, data);
		}

		void input(timer0::source src, uint16_t v = 1) { timer.input(src, v); }

		bool has_reached_last_value() const {
			mask m{0x0000'1000};
			return m(memory[1]);
		}

		bool has_reached_target_value() const {
			mask m{0x0000'0800};
			return m(memory[1]);
		}

		void sync_status(timer0::sync_status_t v) {
			uint32_t data = memory[1];
			mask{0x0000'0001}(data) = v;
			bus.write(4, data);
		}
		void sync_pause_on_hblank() { change_sync_mode(0); }
		void sync_reset_on_hblank() { change_sync_mode(1); }
		void sync_reset_on_hblank_pause_outside() { change_sync_mode(2); }
		void sync_pause_until_hblank() { change_sync_mode(3); }

	  private:
		void change_sync_mode(uint8_t v) {
			uint32_t data = memory[1];
			mask{0x0000'0006}(data) = v;
			bus.write(4, data);
		}

		psycris::bus::data_bus bus;
	};
}

TEST_CASE("Timers common interface", "[hw]") {
	test_timer t0;

	SECTION("A timer has two inputs, but only the active one increases the counter") {
		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 1);

		t0.input(timer0::dot_clock);
		REQUIRE(t0.value() == 1);
	}

	SECTION("When the timer mode is changed (for any reason) the timer value resets to 0") {
		t0.input(timer0::system_clock);
		t0.select_input(timer0::dot_clock);
		REQUIRE(t0.value() == 0);
	}

	SECTION("A timer can count up to 65535, after which it starts again from 0") {
		for (int i = 0; i < 65535; i++) {
			t0.input(timer0::system_clock);
		}
		REQUIRE(t0.value() == 65535);

		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 0);

		SECTION("when the end is reached a sticky signalling bit is set") {
			REQUIRE(t0.has_reached_last_value());

			t0.input(timer0::system_clock);
			REQUIRE(t0.has_reached_last_value());

			SECTION("but the act of reading it resets it out") { REQUIRE(!t0.has_reached_last_value()); }
		}
	}

	SECTION("A timer can also count up to a specified target") {
		t0.target(10);
		t0.input(timer0::system_clock, 10);

		SECTION("when the target is reached a sticky signalling bit is set") {
			REQUIRE(t0.has_reached_target_value());

			t0.input(timer0::system_clock);
			REQUIRE(t0.has_reached_target_value());

			SECTION("but the act of reading it resets it out") { REQUIRE(!t0.has_reached_target_value()); }
		}

		SECTION("reaching the target does not stop the timer") {
			t0.input(timer0::system_clock);
			REQUIRE(t0.value() == 11);

			SECTION("but it can be configured to restart from zero when the target is reached") {
				t0.target(10);
				t0.reset_mode(target_reached);
				t0.input(timer0::system_clock, 10);

				REQUIRE(t0.value() == 0);
			}
		}
	}
}

TEST_CASE("Timer 0 sync modes", "[hw]") {
	test_timer t0;

	SECTION("The counter can be paused during an hblank") {
		t0.sync_status(timer0::enabled);
		t0.sync_pause_on_hblank();

		t0.input(timer0::system_clock);

		t0.timer.enter_hblank();
		t0.input(timer0::system_clock);
		t0.timer.exit_hblank();

		t0.input(timer0::system_clock);

		REQUIRE(t0.value() == 2);
	}

	SECTION("The counter can be reset to 0 when entering an hblank") {
		t0.sync_status(timer0::enabled);
		t0.sync_reset_on_hblank();

		t0.input(timer0::system_clock);

		t0.timer.enter_hblank();
		t0.input(timer0::system_clock);

		REQUIRE(t0.value() == 1);
	}

	SECTION("The counter can be reset to 0 when entering an hblank and paused outside the hblanks") {
		t0.sync_status(timer0::enabled);
		t0.sync_reset_on_hblank_pause_outside();

		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 0);

		t0.timer.enter_hblank();
		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 1);
		t0.timer.exit_hblank();

		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 1);

		t0.timer.enter_hblank();
		REQUIRE(t0.value() == 0);
	}

	SECTION("The counter can be paused until an hblank and then switch to free run") {
		t0.sync_status(timer0::enabled);
		t0.sync_pause_until_hblank();

		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 0);

		t0.timer.enter_hblank();
		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 1);
		REQUIRE(t0.timer.sync_status() == timer0::disabled);
	}
}