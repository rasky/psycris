#pragma once
#include "../logging.hpp"
#include <cstdint>

namespace cpu {
	/**
	 * \brief An interface for a data_port, an hw device connected to the cpu
	 * bus.
	 *
	 * A data port is a device connected to the cpu through the data bus.
	 *
	 * The cpu interacts with this data port only using read or write commands.
	 *
	 * The reads or writes can be one, two or four bytes length and are modelled
	 * by six virtual methods (three for the reads and three for the writes).
	 *
	 * A data port can implement only a subset of these methods, for example
	 * only the write methods if it is a write-only device or only the 8bit
	 * variants; the default implementation log the access attempt and returns a
	 * default value.
	 *
	 * This interface does not describe an hw device in its entirety, but only
	 * the interactions with the cpu. A memory mapped device needs also of a
	 * "size" (how much memory is reserved for it) and of an address (where it
	 * can be found); such infromations are stored in the data bus.
	 *
	 * Since this interface doesn't deal with sizes nor address, the read and
	 * write methods only work with an offset.
	 */
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

namespace cpu {

	struct address_range {
		uint32_t start;
		uint32_t end;

		bool operator&(uint32_t v) const { return v >= start && v <= end; }
	};

	class data_bus {
	  public:
		static const uint32_t open_bus = 0xffff'ffff;

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
		void connect(address_range r, data_port& dp) { ports.push_back({r, &dp}); }

	  public:
		template <typename T>
		T read(uint32_t addr) {
			static_assert(std::is_unsigned_v<T> && sizeof(T) <= 4,
			              "The data_bus read type must be a 8/16/32 unsigned int");

			auto pos = std::find_if(std::begin(ports), std::end(ports), [=](port_map& m) { return m.range & addr; });
			if (pos == std::end(ports)) {
				psycris::log->warn(
				    "[BUS] unmapped read of {} bytes at {:0>8x} ({})", sizeof(T), addr, "guess_io_port(addr)");
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
				psycris::log->warn("[BUS] unmapped write of {} bytes at {:0>8x} ({:0>8x}) ({})",
				                   sizeof(T),
				                   addr,
				                   val,
				                   "guess_io_port(addr)");
				return;
			}

			port_map& m = *pos;
			m.port->write(m.offset(addr), val);
		}

	  private:
		std::vector<port_map> ports;
	};
}