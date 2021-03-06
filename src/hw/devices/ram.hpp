#pragma once
#include "../bus.hpp"
#include "../mmap_device.hpp"

namespace psycris::hw {
	class ram : public mmap_device<ram, 2 * 1024 * 1024> {
	  public:
		static constexpr char const* device_name = "RAM";

		ram(gsl::span<uint8_t, size> buffer) : mmap_device{buffer} {}
	};

	class rom : public mmap_device<rom, 512 * 1024> {
	  public:
		static constexpr char const* device_name = "ROM";

		rom(gsl::span<uint8_t, size> buffer) : mmap_device{buffer} {}
	};
}