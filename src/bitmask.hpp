#pragma once
#include <cstddef>
#include <stdexcept>

namespace psycris {
	struct masked_value {
		constexpr operator uint32_t() const { return value; }

		template <typename T>
		constexpr uint32_t operator=(T) {
			static_assert(sizeof(T) < 0, "this masked value is read-only");
			return 0;
		}

		uint32_t value;
	};

	struct masked_proxy {
		constexpr masked_proxy(uint32_t& v, uint32_t m, uint8_t o) : value{v}, mask{m}, offset{o} {}

		constexpr operator uint32_t() const { return (value & mask) >> offset; }

		constexpr uint32_t operator=(uint32_t v) {
			value = (value & ~mask) | ((v << offset) & mask);
			return v;
		}

		constexpr masked_proxy& operator&=(uint32_t v) {
			value = value & ((v << offset) | ~mask);
			return *this;
		}

		constexpr uint32_t operator<<=(uint8_t b) { return *this = static_cast<uint32_t>(*this) << b; }
		constexpr uint32_t operator>>=(uint8_t b) { return *this = static_cast<uint32_t>(*this) >> b; }

		uint32_t& value;
		uint32_t mask;
		uint8_t offset;
	};

	template <typename Tag>
	struct bit_mask {
		constexpr bit_mask(uint32_t m) : mask{m}, offset{0} {
			if (m == 0) {
				throw std::runtime_error("a bit_mask cannot be empty");
			}

			while ((m & 0x1) == 0) {
				m >>= 1;
				offset++;
			}

			// search for a non-contiguos mask
			while ((m & 0x1) == 1) {
				m >>= 1;
			}
			if (m != 0) {
				throw std::runtime_error("non-contiguos masks are not supported");
			}
		}

		constexpr masked_value operator()(uint32_t const& v) const { return {(v & mask) >> offset}; }

		constexpr masked_proxy operator()(uint32_t& v) const { return {v, mask, offset}; }

		uint32_t mask;
		uint8_t offset;
	};
}