#pragma once
#include <cstddef>
#include <stdexcept>

namespace psycris {
	template <typename ValueType>
	class masked_value {
	  public:
		masked_value(ValueType v) : value{v} {}

		constexpr operator ValueType() const { return value; }

		template <typename T>
		constexpr ValueType operator=(T) {
			static_assert(sizeof(T) < 0, "this masked value is read-only");
			return 0;
		}

	  private:
		ValueType value;
	};

	template <typename ValueType>
	class masked_proxy {
	  public:
		constexpr masked_proxy(ValueType& v, ValueType m, uint8_t o) : value{v}, mask{m}, offset{o} {}

	  public:
		constexpr operator ValueType() const { return (value & mask) >> offset; }

		constexpr ValueType operator=(ValueType v) {
			value = (value & ~mask) | ((v << offset) & mask);
			return v;
		}

		constexpr masked_proxy& operator&=(ValueType v) {
			value = value & ((v << offset) | ~mask);
			return *this;
		}

		constexpr ValueType operator<<=(uint8_t b) { return *this = static_cast<ValueType>(*this) << b; }
		constexpr ValueType operator>>=(uint8_t b) { return *this = static_cast<ValueType>(*this) >> b; }

	  private:
		ValueType& value;
		ValueType mask;
		uint8_t offset;
	};

	template <typename Tag, typename ValueType = uint32_t>
	struct bit_mask {
		static_assert(std::is_unsigned_v<ValueType>, "The bitmask type must be unsigned");

		constexpr bit_mask(ValueType m) : mask{m}, offset{0} {
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

		constexpr masked_value<ValueType> operator()(ValueType const& v) const {
			return masked_value<ValueType>((v & mask) >> offset);
		}

		constexpr masked_proxy<ValueType> operator()(ValueType& v) const { return {v, mask, offset}; }

		ValueType mask;
		uint8_t offset;
	};
}