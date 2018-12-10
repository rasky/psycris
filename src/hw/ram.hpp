#pragma once
#include "../cpu/bus.hpp"
#include "memory_range.hpp"

#include <cstdint>
#include <cstdlib>
#include <gsl/span>

namespace psycris::hw {
	class ram : public cpu::data_port, public memory_range<2 * 1024 * 1024> {
	  public:
		using memory_range::memory_range;

	  private:
		uint8_t read8(uint32_t offset) const override {
			return memory[offset];
		}

		uint16_t read16(uint32_t offset) const override {
			return *(reinterpret_cast<uint16_t*>(&memory[offset]));
		}

		uint32_t read32(uint32_t offset) const override {
			return *(reinterpret_cast<uint32_t*>(&memory[offset]));
		}

	  private:
		void write8(uint32_t offset, uint8_t val) override {
			memory[offset] = val;
		}

		void write16(uint32_t offset, uint16_t val) override {
			*(reinterpret_cast<uint16_t*>(&memory[offset])) = val;
		}

		void write32(uint32_t offset, uint32_t val) override {
			*(reinterpret_cast<uint32_t*>(&memory[offset])) = val;
		}
	};

	class rom : public cpu::data_port, public memory_range<512 * 1024> {
	  public:
		using memory_range::memory_range;

	  private:
		uint8_t read8(uint32_t offset) const override {
			return memory[offset];
		}

		uint16_t read16(uint32_t offset) const override {
			return *(reinterpret_cast<uint16_t*>(&memory[offset]));
		}

		uint32_t read32(uint32_t offset) const override {
			return *(reinterpret_cast<uint32_t*>(&memory[offset]));
		}
	};
}