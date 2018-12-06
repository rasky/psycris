#pragma once
#include "../logging.hpp"
#include "decoder.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <gsl/span>
#include <type_traits>

namespace cpu {
	class data_port {
	  public:
		template <typename T>
		T read(uint32_t offset) {
			assert(sizeof(T) <= 4);

			if constexpr (sizeof(T) == 1) {
				return read8(offset);
			} else if constexpr (sizeof(T) == 2) {
				return read16(offset);
			} else {
				return read32(offset);
			}
		}

		template <typename T>
		void write(uint32_t offset, T val) {
			assert(sizeof(T) <= 4);

			if constexpr (sizeof(T) == 1) {
				write8(offset, val);
			} else if constexpr (sizeof(T) == 2) {
				write16(offset, val);
			} else {
				write32(offset, val);
			}
		}

	  private:
		virtual uint8_t read8(uint32_t offset) const {
			psycris::log->warn("unsupported read of 1 byte at offset {:0>8x}. Is this a write-only port?", offset);
			return 0;
		}

		virtual uint16_t read16(uint32_t offset) const {
			psycris::log->warn("unsupported read of 2 bytes at offset {:0>8x}. Is this a write-only port?", offset);
			return 0;
		}

		virtual uint32_t read32(uint32_t offset) const {
			psycris::log->warn("unsupported read of 4 bytes at offset {:0>8x}. Is this a write-only port?", offset);
			return 0;
		}

	  private:
		virtual void write8(uint32_t offset, uint8_t val) {
			psycris::log->warn(
			    "unsupported write of 1 byte at offset {:0>8x} ({:0>8x}). Is this a read-only port?", offset, val);
		}

		virtual void write16(uint32_t offset, uint16_t val) {
			psycris::log->warn(
			    "unsupported write of 2 bytes at offset {:0>8x} ({:0>8x}). Is this a read-only port?", offset, val);
		}

		virtual void write32(uint32_t offset, uint32_t val) {
			psycris::log->warn(
			    "unsupported write of 4 bytes at offset {:0>8x} ({:0>8x}). Is this a read-only port?", offset, val);
		}
	};
}

namespace hw {

	template <size_t Size>
	struct memory_range {
		static constexpr size_t size = Size;

		memory_range(gsl::span<uint8_t, size> buffer) : memory{buffer} {
		}

		gsl::span<uint8_t, size> memory;
	};

	class ram : public cpu::data_port, memory_range<2 * 1024 * 1024> {
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
}

namespace cpu {

	struct address_range {
		uint32_t start;
		uint32_t end;

		bool operator&(uint32_t v) const {
			return v >= start && v <= end;
		}
	};

	class data_busX {
	  public:
		static const uint32_t open_bus = 0xffff'ffff;

	  public:
		void connect(address_range, data_port&);

	  private:
		struct port_map {
			address_range range;
			data_port* port;

			uint32_t offset(uint32_t addr) const {
				assert(addr <= range.end);
				return addr - range.start;
			}
		};

	  public:
		template <typename T>
		T read(uint32_t addr) {
			static_assert(std::is_unsigned_v<T> && sizeof(T) <= 4,
			              "The data_bus read type must be a 8/16/32 unsigned int");

			auto pos = std::find_if(std::begin(ports), std::end(ports), [=](port_map& m) { return m.range & addr; });
			if (pos == std::end(ports)) {
				psycris::log->warn("[BUS] unmapped read of {} bytes at {:0>8x}", sizeof(T), addr);
				return static_cast<T>(open_bus);
			}

			port_map& m = *pos;
			return m.port->read<T>(m.offset(addr));
		}

		template <typename T>
		void write(uint32_t addr, T val) {
			static_assert(std::is_unsigned_v<T> && sizeof(T) <= 4,
			              "The data_bus write type must be a 8/16/32 unsigned int");

			auto pos = std::find_if(std::begin(ports), std::end(ports), [=](port_map& m) { return m.range & addr; });
			if (pos == std::end(ports)) {
				psycris::log->warn("[BUS] unmapped write of {} bytes at {:0>8x} ({:0>8x})", sizeof(T), addr, val);
				return;
			}

			port_map& m = *pos;
			m.port->write(m.offset(addr), val);
		}

	  private:
		std::vector<port_map> ports;
	};
}
namespace cpu {
	/**
	 * \brief given an IO addres returns a string describing its use
	 *
	 * This function is intended to be used during the debug only.
	 */
	std::string guess_io_port(uint32_t addr);

	/**
	 * \brief CPU data bus
	 *
	 *
	 * https://problemkaputt.de/psx-spx.htm#memorymap
	 *
	 * Memory Map
	 *
	 *   KUSEG     KSEG0     KSEG1
	 *   00000000h 80000000h A0000000h  2048K  Main RAM (first 64K reserved for BIOS)
	 *   1F000000h 9F000000h BF000000h  8192K  Expansion Region 1 (ROM/RAM)
	 *   1F800000h 9F800000h    --      1K     Scratchpad (D-Cache used as Fast RAM)
	 *   1F801000h 9F801000h BF801000h  8K     I/O Ports
	 *   1F802000h 9F802000h BF802000h  8K     Expansion Region 2 (I/O Ports)
	 *   1FA00000h 9FA00000h BFA00000h  2048K  Expansion Region 3 (whatever purpose)
	 *   1FC00000h 9FC00000h BFC00000h  512K   BIOS ROM (Kernel) (4096K max)
	 *         FFFE0000h (KSEG2)        0.5K   I/O Ports (Cache Control)
	 *
	 */
	class data_bus {
	  public:
		data_bus(gsl::span<uint8_t> bios, gsl::span<uint8_t> ram) : bios{bios}, ram{ram} {
		}

	  public:
		static const uint32_t open_bus = 0xffff'ffff;

	  public:
		template <typename T>
		T read(uint32_t addr) {
			static_assert(std::is_unsigned_v<T> && sizeof(T) <= 4,
			              "The data_bus read type must be a 8/16/32 unsigned int");

			using psycris::log;

			uint8_t hw = addr >> 24;
			if (hw < 0x1f || (hw >= 0x80 && hw < 0x9f) || (hw >= 0xa0 && hw < 0xbf)) {
				// RAM
				return *(reinterpret_cast<T*>(&ram[addr & 0xff'ffff]));
			}

			switch (addr >> 20) {
			case 0x1FC:
			case 0x9FC:
			case 0xBFC:
				// BIOS
				// undefined behaviour?
				return *(reinterpret_cast<T*>(&bios[addr & 0xf'ffff]));
				break;
			}

			log->warn("[BUS] unmapped read at {:0>8x} ({} bytes)", addr, sizeof(T));
			return static_cast<T>(open_bus);
		}

		template <typename T>
		void write(uint32_t addr, T val) {
			static_assert(std::is_unsigned_v<T> && sizeof(T) <= 4,
			              "The data_bus write type must be a 8/16/32 unsigned int");

			using psycris::log;

			switch (addr >> 24) {
			case 0x00:
			case 0x80:
			case 0xa0:
				log->trace("RAM:{:0>8x} << {:#x}", addr, val);
				*(reinterpret_cast<T*>(&ram[addr & 0xff'ffff])) = val;
				break;
			default:
				std::string msg = fmt::format(
				    "[BUS] unmapped write at {{:0>8x}}, 0x{{:0>{}x}} ({})", sizeof(val) * 2, guess_io_port(addr));
				log->warn(msg.c_str(), addr, val);
			}
		}

	  public:
		gsl::span<uint8_t> ram_slice(uint32_t addr, uint32_t size) {
			uint8_t hw = addr >> 24;

			bool in_ram = hw < 0x1f || (hw >= 0x80 && hw < 0x9f) || (hw >= 0xa0 && hw < 0xbf);
			assert(in_ram);

			uint32_t offset = addr & 0x00ff'ffff;
			assert(offset + size <= ram.size());

			return {&ram[offset], size};
		}

		gsl::span<uint8_t> bios_slice(uint32_t addr, uint32_t size) {
			uint16_t hw = addr >> 20;

			bool in_bios = hw == 0x1fc || hw == 0x9fc || hw == 0xbfc;
			assert(in_bios);

			uint32_t offset = addr & 0x000f'ffff;
			assert(offset + size <= bios.size());

			return {&bios[offset], size};
		}

	  public:
		gsl::span<uint8_t> bios;
		gsl::span<uint8_t> ram;
	};
}