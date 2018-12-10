#pragma once
#include <gsl/span>

namespace psycris::hw {
	template <size_t Bytes>
	struct memory_range {
		static constexpr size_t size = Bytes;

		memory_range(gsl::span<uint8_t, size> buffer) : memory{buffer} {
		}

		gsl::span<uint8_t, size> memory;
	};
}