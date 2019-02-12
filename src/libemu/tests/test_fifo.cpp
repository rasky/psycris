#include <catch2/catch.hpp>

#include "../fixed_fifo.hpp"

TEST_CASE("A fixed_fifo is a FIFO implemented with a circluar buffer", "[core]") {
	psycris::fixed_fifo<int, 2> input;

	REQUIRE(input.capacity() == 2);
	REQUIRE(input.size() == 0);
	REQUIRE(input.empty() == true);
	REQUIRE(input.full() == false);

	SECTION("new values can be pushed up to the configured limit") {
		input.push(0);
		REQUIRE(input.empty() == false);

		input.push(1);
		REQUIRE(input.full() == true);

		SECTION("pushing another value in a full fifo raises an exception") { REQUIRE_THROWS(input.push(2)); }
	}

	SECTION("values are popped in the insertion order") {
		input.push(0);
		input.push(1);

		REQUIRE(input.pop() == 0);
		REQUIRE(input.full() == false);

		REQUIRE(input.pop() == 1);
		REQUIRE(input.empty() == true);

		SECTION("popping from an empty fifo raises an exception") { REQUIRE_THROWS(input.pop()); }
	}

	SECTION("popping an element makes room to more values") {
		input.push(0);
		input.push(1);

		input.pop();
		input.push(2);

		input.pop();

		int expected = 2;
		int steps = input.capacity() * 3;
		while (steps-- > 0) {
			input.push(expected + 1);
			REQUIRE(input.pop() == expected);
			expected++;
		}
	}
}