#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

#include <gsl/span>
#include <vector>

#include "../logging.hpp"

/**
 * \brief The `bus` namespace contains the type `data_bus` and the interfaces
 * needed to implement the devices to connect to.
 *
 * There are three main types in this namespace:
 * - data_bus
 * - device
 * - data_port
 *
 * The `data_bus` is a concrete type that models the bus that connect a cpu with
 * the board devices. It has three main methods: `connect`, `read` and `write`.
 * The first one is used to connect all the devices to the bus, while the other
 * two are normally used by a cpu.
 *
 * The `device` is a virtual class that defines the interface that a device must
 * implement to be connectable to a bus.
 *
 * A `device` can be logically divided into "data ports"; a `data_port` is a sub
 * part of a device, it is placed at an offset from the beginning of the device
 * and it's size can only be 1, 2 or 4 bytes.
 *
 * The method `data_port::post_write` is called by the bus whenever the
 * data_port memory is written
 */
namespace psycris::bus {
	/**
	 * \brief given an IO addres returns a string describing its use
	 *
	 * This function is intended to be used during the debug only.
	 */
	std::string guess_io_port(uint32_t addr);

	class data_port {
	  public:
		data_port(uint8_t offset, uint8_t size) : _offset{offset}, _size{size} {}

		virtual ~data_port() = default;

	  public:
		uint8_t offset() const { return _offset; }
		uint8_t size() const { return _size; }

	  public:
		/**
		 * \brief called by the `data_bus` when a write involves this port
		 *
		 * When this method is called, the device memory has already been
		 * updated.
		 *
		 * \param new_value the new value for this port
		 * \param old_value the previous value of this port
		 *
		 * The type for the two values is always `uint32_t` even if the write
		 * was smaller (in this case the upper bits of the values are set to
		 * zero). This decision was made to only require a single virtual
		 * method.
		 */
		virtual void post_write(uint32_t new_value, uint32_t old_value) const = 0;

	  private:
		uint8_t _offset;
		uint8_t _size;
	};

	class device {
	  public:
		virtual ~device() = default;

	  public:
		/**
		 * \brief returns the device memory
		 */
		virtual gsl::span<uint8_t> memory() const = 0;

		/**
		 * \brief returns this device data ports
		 *
		 * \return std::vector<bus::data_port const*> const& with the device
		 * ports sorted by their offset.
		 */
		virtual std::vector<bus::data_port const*> const& ports() = 0;
	};

	struct address_range {
		uint32_t start;
		uint32_t end;

		bool operator&(uint32_t v) const { return v >= start && v <= end; }
	};

	class data_bus {
	  public:
		/**
		 * \brief the value returned by an unmapped read
		 */
		static const uint32_t open_bus = 0xffff'ffff;

	  private:
		struct device_map {
			address_range range;
			device* d;

			uint32_t offset(uint32_t addr) const {
				assert(addr <= range.end);
				return addr - range.start;
			}
		};

	  public:
		/**
		 * \brief connects a new device to the bus
		 *
		 * The device is mapped to the specified address range. The same device
		 * can be mapped multiple times to different ranges.
		 */
		void connect(address_range r, device& dp) { devices.push_back({r, &dp}); }

		void connect(uint32_t start, device& dp) {
			connect({start, gsl::narrow_cast<uint32_t>(start + dp.memory().size())}, dp);
		}

	  public:
		template <typename T>
		T read(uint32_t addr) {
			static_assert(std::is_unsigned_v<T> && sizeof(T) <= 4,
			              "The data_bus read type must be a 8/16/32 unsigned int");

			auto device = find_device(addr);
			if (!device) {
				psycris::log->warn(
				    "[BUS] unmapped read of {} bytes at {:0>8x} ({})", sizeof(T), addr, guess_io_port(addr));
				return static_cast<T>(open_bus);
			}

			return read<T>(*device, addr);
		}

		template <typename T>
		void write(uint32_t addr, T val) {
			static_assert(std::is_unsigned_v<T> && sizeof(T) <= 4,
			              "The data_bus write type must be a 8/16/32 unsigned int");

			auto device = find_device(addr);
			if (!device) {
				psycris::log->warn("[BUS] unmapped write of {} bytes at {:0>8x} ({:0>8x}) ({})",
				                   sizeof(T),
				                   addr,
				                   val,
				                   guess_io_port(addr));
				return;
			}

			T prev_value = read<T>(*device, addr);
			write(*device, addr, val);
			touch_data_ports<T>(*device, addr, prev_value);
		}

	  private:
		device_map const* find_device(uint32_t addr) const {
			auto pos = std::find_if(                    //
			    std::begin(devices),                    //
			    std::end(devices),                      //
			    [=](auto& m) { return m.range & addr; } //
			);
			return pos == std::end(devices) ? nullptr : &(*pos);
		}

		template <typename T>
		T read(device_map const& map, uint32_t addr) const {
			T value;
			auto device_memory = map.d->memory();
			std::memcpy(&value, &device_memory[map.offset(addr)], sizeof(T));
			return value;
		}

		uint32_t read(device_map const& map, data_port const& port) const {
			uint32_t value;
			auto device_memory = map.d->memory();
			std::memcpy(&value, &device_memory[port.offset()], sizeof(value));
			if (port.size() == 1) {
				value &= 0x0000'00ff;
			} else if (port.size() == 2) {
				value &= 0x0000'ffff;
			}
			return value;
		}

		template <typename T>
		void write(device_map const& map, uint32_t addr, T value) {
			auto device_memory = map.d->memory();
			std::memcpy(&device_memory[map.offset(addr)], &value, sizeof(T));
		}

		template <typename T>
		void touch_data_ports(device_map const& map, uint32_t addr, T overwritten_value) {
			int8_t written_bytes = sizeof(T);

			// where the write starts from the device POV
			uint32_t device_offset = map.offset(addr);

			// a char pointer to the overwritten value; it is used to recover
			// the old value for every port
			auto over = reinterpret_cast<char*>(&overwritten_value);

			for (auto& port : map.d->ports()) {
				uint32_t next_port_offset = port->offset() + port->size();

				bool touched = device_offset >= port->offset() && device_offset < next_port_offset;
				if (!touched) {
					continue;
				}

				// how many bytes of this port are touched by this write
				uint8_t port_bytes = std::min(gsl::narrow_cast<int8_t>(next_port_offset - device_offset),
				                              written_bytes);
				{
					uint32_t new_value = read(map, *port);
					uint32_t old_value = new_value;

					// `p` points to the overwritten portion of `old_value`
					auto p = reinterpret_cast<char*>(&old_value) + (device_offset - port->offset());
					std::memcpy(p, over, port_bytes);

					port->post_write(new_value, old_value);
				}

				written_bytes -= port_bytes;
				if (written_bytes <= 0) {
					break;
				}

				over += port_bytes;
				device_offset = next_port_offset;
			}

			// if (written_bytes > 0) {
			// 	assert(!"TODO implement a multi-device write");
			// }
		}

	  private:
		std::vector<device_map> devices;
	};
}