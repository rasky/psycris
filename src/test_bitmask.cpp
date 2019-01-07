#include <catch2/catch.hpp>

#include "bitmask.hpp"

namespace {
	using mask = psycris::bit_mask<class test_>;
	constexpr mask flag1{0x0000'0f00}; // 4 bit
	constexpr mask flag2{0x0000'0080}; // 1 bit
	constexpr mask flag3{0x0000'007c}; // 5 bit
}

TEST_CASE("a bitmask makes it easier to work with a bitset", "[core]") {
	uint32_t value = 0xffff'ffff;

	SECTION("it can read the associated portion") {
		REQUIRE(flag1(value) == 15);
		REQUIRE(flag2(value) == 1);
		REQUIRE(flag3(value) == 31);
	}

	SECTION("it can change the associated portion") {
		SECTION("by overwriting the old value") {
			flag3(value) = 0;
			REQUIRE(flag3(value) == 0);
			REQUIRE(value == 0xffff'ff83);
		}

		SECTION("by shifting the old value") {
			SECTION("the shift to the left inserts zeros") {
				flag3(value) <<= 2;
				REQUIRE(flag3(value) == 0x1c);
				REQUIRE(value == 0xffff'fff3);
			}

			SECTION("the shift to the right inserts zeros") {
				flag3(value) >>= 2;
				REQUIRE(flag3(value) == 0x07);
				REQUIRE(value == 0xffff'ff9f);
			}
		}
	}
}
