#include <catch2/catch.hpp>

#include "../bitmask.hpp"
#include "../devices/timer.hpp"

using namespace psycris::hw;

namespace {
	enum reset_mode {
		end_reached = 0,
		target_reached = 1,
	};

	struct test_timer {
		std::array<uint32_t, 3> memory;

		timer0 timer;

		int interrupts;

		test_timer()
		    : memory{},
		      timer{gsl::span{reinterpret_cast<uint8_t*>(memory.data()), 12}, [this]() { this->interrupts += 1; }},
		      interrupts{0} {
			select_input(timer0::system_clock);
			target(0);
			reset_mode(end_reached);
		}

		using mask = psycris::bit_mask<class mask_>;

		uint16_t value() const { return timer.value(); }

		void target(uint16_t v) { memory[2] = v; }

		void reset_mode(reset_mode v) {
			mask m{0x0000'0008};
			m(memory[1]) = static_cast<uint8_t>(v);
		}

		void select_input(timer0::source src) {
			mask m{0x0000'0300};
			m(memory[1]) = static_cast<uint8_t>(src);
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
	};
}

TEST_CASE("Timer tests", "[hw]") {
	test_timer t0;

	SECTION("A timer has two inputs, but only the active one increases the counter") {
		t0.input(timer0::system_clock);
		REQUIRE(t0.value() == 1);

		t0.input(timer0::dot_clock);
		REQUIRE(t0.value() == 1);
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
				t0.target(12);
				t0.reset_mode(target_reached);
				t0.input(timer0::system_clock);

				REQUIRE(t0.value() == 0);
			}
		}
	}
}