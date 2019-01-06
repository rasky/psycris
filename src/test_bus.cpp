#include <catch2/catch.hpp>

#include "hw/bus.hpp"
#include "hw/mmap_device.hpp"

namespace {
	namespace hw = psycris::hw;

	using reg1 = hw::data_reg<0>;
	using reg2 = hw::data_reg<4, 2>;
	using reg3 = hw::data_reg<6, 2>;
	using reg4 = hw::data_reg<8>;

	struct controller : hw::mmap_device<controller, 12> {
		controller(gsl::span<uint8_t, size> buffer) : mmap_device{buffer, reg1{}, reg2{}, reg3{}, reg4{}} {}

		void wcb(reg1, uint32_t value) { writes[0].push_back(value); }
		void wcb(reg2, uint32_t value) { writes[1].push_back(value); }
		void wcb(reg3, uint32_t value) { writes[2].push_back(value); }
		void wcb(reg4, uint32_t value) { writes[4].push_back(value); }

		std::array<std::vector<uint32_t>, 4> writes = {};
	};

	struct ram : hw::mmap_device<ram, 10> {
		ram(gsl::span<uint8_t, size> buffer) : mmap_device{buffer} {}
	};

	struct test_board {
		// This is all the board memory; every devices have a slice of this
		// buffer.
		std::array<uint8_t, controller::size + ram::size> memory;

		// This test board is composed of two devices: a "controller" device
		// (with 4 data ports) and a buffer (ram) of 10 bytes.
		controller ctrl;
		ram buffer;

		// the board data bus
		psycris::bus::data_bus bus;

		const size_t ctrl_addr = 0x1000'0000;
		const size_t buffer_addr = 0x1000'0000 + controller::size;
		const size_t ctrl_alias_addr = 0x2000'0000;

		test_board() : memory{}, ctrl{{memory.data(), 12}}, buffer{{memory.data() + 12, 10}} {
			for (size_t ix = 0; ix < memory.size(); ix++) {
				memory[ix] = ix;
			}

			bus.connect(ctrl_addr, ctrl);
			bus.connect(ctrl_alias_addr, ctrl);
			bus.connect(buffer_addr, buffer);
		}
	};
}

TEST_CASE("data_bus read operations", "[bus]") {
	test_board board;
	auto& bus = board.bus;

	SECTION("the same location can be read 8, 16 or 32 bits at a time") {
		REQUIRE(bus.read<uint8_t>(board.ctrl_addr) == 0x00);
		REQUIRE(bus.read<uint16_t>(board.ctrl_addr) == 0x0100);
		REQUIRE(bus.read<uint32_t>(board.ctrl_addr) == 0x0302'0100);
	}

	SECTION("an unmapped location can be read, the returned value is the 'open_bus' constant") {
		REQUIRE(bus.read<uint32_t>(0) == 0xffff'ffff);
	}

	SECTION("the same device can be mapped multiple times at different addresses") {
		REQUIRE(bus.read<uint32_t>(board.ctrl_addr) == bus.read<uint32_t>(board.ctrl_alias_addr));
	}

	SECTION("the bus can read from any address ignoring the device layout") {
		SECTION("it can read a data port") {                                       //
			REQUIRE(bus.read<uint16_t>(board.ctrl_addr + reg2::offset) == 0x0504); //
		}                                                                          //

		SECTION("it can read a part of a data port") {
			REQUIRE(bus.read<uint8_t>(board.ctrl_addr + reg2::offset) == 0x04);
		}

		SECTION("it can read multiple data ports") {
			REQUIRE(bus.read<uint32_t>(board.ctrl_addr + reg2::offset) == 0x0706'0504);
		}

		SECTION("it can read across multiple data ports") {
			REQUIRE(bus.read<uint32_t>(board.ctrl_addr + reg2::offset - 1) == 0x0605'0403);
		}

		SECTION("it can read across multiple devices") {
			REQUIRE(bus.read<uint32_t>(board.ctrl_addr + reg4::offset + 2) == 0x0d0c'0b0a);
		}
	}
}

TEST_CASE("data_bus write operations", "[bus]") {
	test_board board;
	auto& bus = board.bus;

	SECTION("the same location can be written 8, 16 or 32 bits at a time") {
		bus.write(board.ctrl_addr, static_cast<uint8_t>(0xff));
		REQUIRE(bus.read<uint32_t>(board.ctrl_addr) == 0x0302'01ff);

		bus.write(board.ctrl_addr, static_cast<uint16_t>(0xffff));
		REQUIRE(bus.read<uint32_t>(board.ctrl_addr) == 0x0302'ffff);

		bus.write(board.ctrl_addr, static_cast<uint32_t>(0xffff'ffff));
		REQUIRE(bus.read<uint32_t>(board.ctrl_addr) == 0xffff'ffff);
	}

	SECTION("an unmapped location can be written, but it is a no-op") {
		bus.write(0, static_cast<uint32_t>(0x0000'0000));
		REQUIRE(bus.read<uint32_t>(0) == 0xffff'ffff);
	}

	SECTION("the same device can be mapped multiple times at different addresses") {
		bus.write(board.ctrl_addr, static_cast<uint32_t>(0xffff'ffff));
		REQUIRE(bus.read<uint32_t>(board.ctrl_alias_addr) == 0xffff'ffff);
	}

	SECTION("the bus can inform a device of each write made") {
		uint16_t v = 0xaa00;

		bus.write(board.ctrl_addr + reg3::offset, v);
		REQUIRE(board.ctrl.writes[2].size() == 1);
		REQUIRE(board.ctrl.writes[2].back() == v);

		v += 1;
		bus.write(board.ctrl_addr + reg3::offset, v);
		REQUIRE(board.ctrl.writes[2].size() == 2);
		REQUIRE(board.ctrl.writes[2].back() == v);

		SECTION("the device is informed for every port written") {
			uint32_t v = 0xdead'beef;
			bus.write(board.ctrl_addr + reg2::offset - 1, v);
			REQUIRE(board.ctrl.writes[0].back() == 0xef02'0100);
			REQUIRE(board.ctrl.writes[1].back() == 0xadbe);
			REQUIRE(board.ctrl.writes[2].back() == 0xaade);
		}
	}
}