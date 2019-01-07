#pragma once
#include "../cpu/bus.hpp"
#include "../cpu/cop0.hpp"
#include "memory_range.hpp"

#include "../meta.hpp"

#include <fmt/format.h>

namespace psycris::hw {
	/*
#include <array>
#include <boost/hana.hpp>
#include <cstdint>
#include <stdexcept>

namespace hana = boost::hana;
#include <cassert>
template <typename X>
class Q;

// struct data_reg {
//    size_t offset;
// };

// constexpr data_reg control{0};
// constexpr data_reg interrupts{0};

// template <typename D, size_t Bytes, size_t NRegs>
// struct M {

//     std::array<data_reg, NRegs> regs;
// };

// struct D : M<D, 0, 2> {
//   bool reg(control) { return true; }
//   bool reg(control, uint32_t v) { return true; }

//   bool reg(interrupts) { return true; }
//   bool reg(interrupts, uint32_t v) { return true; }
// };


template <size_t Offset>
struct data_reg {
  static constexpr size_t offset = Offset;
};

using control = data_reg<0>;
using interrupts = data_reg<4>;

template <typename T, typename U>
auto is_r(U x) -> decltype(std::declval<T>().reg(x));

namespace details {
	constexpr auto offset = [](auto reg) {
	    using T = typename decltype(reg)::type;
	    return hana::int_c<T::offset>;
	};

	template <typename... Regs>
	struct regs {
	    static constexpr auto types = hana::sort.by(
	        hana::ordering(offset),
	        hana::tuple_t<Regs...>);

	    template <typename M>
	    uint32_t read(M const& memory, size_t offset, size_t bytes) const {
	        uint32_t value;

	        auto ptr = std::begin(memory);
	        hana::for_each(types, [&](auto reg) {
	            using DataReg = typename decltype(reg)::type;
	            if (offset >= DataReg::offset && offset <= DataReg::offset + DataReg::size) {
	                value = *ptr;
	            }
	            ptr++;
	        });

	        return value;
	    }
	};
}

template <typename Derived, size_t Bytes, typename... Regs>
class M {
	public:
	static constexpr auto regs = details::regs<Regs...>{};
 public:
  uint32_t read(size_t offset, size_t size) const {
	  return regs.read(memory, offset, size);
  }
  // void write()
  std::array<uint8_t, Bytes> memory;
};

class D : public M<D, 8, interrupts, control> {
 public:
  uint32_t w(control, uint32_t v) { return v + 1; }
};
	*/
	template <uint8_t Offset, uint8_t Bytes = 4>
	struct data_reg {
		static constexpr uint8_t offset = Offset;
		static constexpr uint8_t size = Bytes;
	};

	namespace details {
		constexpr auto get_offset = [](auto reg) {
			using T = typename decltype(reg)::type;
			return hana::int_c<T::offset>;
		};

		template <typename... Registers>
		struct regs {
			static constexpr auto types = hana::sort.by(hana::ordering(get_offset), hana::tuple_t<Registers...>);

			// uint32_t read(gsl::span<uint8_t> memory, size_t offset, size_t bytes) const {
			// 	uint32_t value;

			// 	auto ptr = std::begin(memory);
			// 	hana::for_each(types, [&](auto reg) {
			// 		using DataReg = typename decltype(reg)::type;
			// 		if (offset >= DataReg::offset && offset <= DataReg::offset + DataReg::size) {
			// 			value = *ptr;
			// 		}
			// 		ptr++;
			// 	});

			// 	return value;
			// }
		};
	}

	template <typename Derived, size_t MemoryBytes, typename... DataPorts>
	struct memory_rangeX {
		static constexpr size_t size = MemoryBytes;

		static constexpr auto regs = details::regs<DataPorts...>{};

		memory_rangeX(gsl::span<uint8_t, size> buffer) : memory{buffer} {}

		gsl::span<uint8_t, size> memory;

	  public:
		uint8_t r8(uint32_t offset) const { return *(reinterpret_cast<uint8_t*>(&memory[offset])); }

		uint16_t r16(uint32_t offset) const {
			uint16_t value;
			std::memcpy(&value, &memory[offset], 2);
			return value;
		}

		uint32_t r32(uint32_t offset) const {
			uint32_t value;
			std::memcpy(&value, &memory[offset], 4);
			return value;
		}

	  public:
		void w8(uint32_t offset, uint8_t value) { write(memory, offset, 1, value); }
		void w16(uint32_t offset, uint16_t value) { write(memory, offset, 2, value); }
		void w32(uint32_t offset, uint32_t value) { write(memory, offset, 4, value); }

	  private:
		uint32_t write(size_t offset, size_t bytes, uint32_t value) {
			hana::for_each(regs.types, [&](auto port) {
				using DataPort = typename decltype(port)::type;
				bool touched = offset >= DataPort::offset && offset <= DataPort::offset + DataPort::size;
				if (!touched)
					return;

				// uint32_t value =
			});
		}
	};

	namespace dma_regs {
		using control = data_reg<0>;
		using interrupt = data_reg<4>;
	}

	class dma : public cpu::data_port, public memory_rangeX<dma, 8, dma_regs::control, dma_regs::interrupt> {
	  public:
		dma(gsl::span<uint8_t, size> buffer) : memory_rangeX(buffer) {
			// write32(0x0765'4321);
		}

	  public:
		enum regs {
			control = 0,
			interrupt = 4,
		};
		// constexpr static uint32_t i_stat = 0;
		// constexpr static uint32_t i_mask = 0;

		// uint32_t read32(uint32_t offset) const override { return *(reinterpret_cast<uint32_t*>(&memory[offset])); }

		// void write32(uint32_t offset, uint32_t val) override {
		// 	uint32_t p = read32(i_stat);
		// 	*(reinterpret_cast<uint32_t*>(&memory[offset])) = val;

		// 	if (offset == i_stat) {
		// 		p = (p ^ val) & read32(i_mask);
		// 		if (p) {
		// 			_cop0->interrupt_request();
		// 		}
		// 	}
		// }
	};
}