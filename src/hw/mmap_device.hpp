#pragma once
#include "../meta.hpp"
#include "./bus.hpp"
#include <cstdint>

namespace psycris::hw {

	template <uint8_t Offset, uint8_t Bytes = 4>
	struct data_reg {
		static_assert(Bytes == 1 || Bytes == 2 || Bytes == 4, "Unsupported port size");

		static constexpr uint8_t offset = Offset;
		static constexpr uint8_t size = Bytes;

		using int_type = std::conditional_t<Bytes == 4, uint32_t, std::conditional_t<Bytes == 2, uint16_t, uint8_t>>;
	};

	namespace details {
		constexpr auto ports_overlaps = [](auto ports) -> bool {
			int last_offset = 0;
			bool overlaps = false;
			hana::for_each(ports, [&](auto port) {
				using T = typename decltype(port)::type;
				if (T::offset < last_offset) {
					overlaps = true;
				}
				last_offset = T::offset + T::size;
			});
			return overlaps;
		};
	}

	template <typename Derived, size_t MemoryBytes>
	class mmap_device : public bus::device {
	  private:
		template <typename DataPort>
		struct mmap_data_port : public bus::data_port {
			__attribute__((no_sanitize("vptr"))) mmap_data_port(mmap_device* d)
			    : bus::data_port{DataPort::offset, DataPort::size}, _device{static_cast<Derived*>(d)} {}

			void post_write([[maybe_unused]] uint32_t new_value, [[maybe_unused]] uint32_t old_value) override {
				auto has_write_callback = hana::is_valid(
				    [](auto&& port) -> decltype(std::declval<Derived>().wcb(port, 0, 0)) {});

				if constexpr (has_write_callback(DataPort{})) {
					_device->wcb(DataPort{}, new_value, old_value);
				}
			}

			Derived* _device;
		};

	  public:
		static constexpr size_t size = MemoryBytes;

		template <typename... DataPorts>
		mmap_device(gsl::span<uint8_t, size> buffer, DataPorts...) : _memory{buffer} {
			if constexpr (sizeof...(DataPorts) > 0) {
				initialize_ports<DataPorts...>();
			}
		}

	  public:
		gsl::span<uint8_t> memory() const override { return _memory; }

		std::vector<std::unique_ptr<bus::data_port>> const& ports() const override { return _ports; }

	  protected:
		template <typename DataPort>
		auto read() const {
			typename DataPort::int_type val;
			std::memcpy(&val, _memory.data() + DataPort::offset, DataPort::size);
			return val;
		}

	  private:
		template <typename... DataPorts>
		void initialize_ports() {
			constexpr auto get_offset = [](auto reg) {
				using T = typename decltype(reg)::type;
				return hana::int_c<T::offset>;
			};
			constexpr auto types = hana::sort.by(hana::ordering(get_offset), hana::tuple_t<DataPorts...>);

			constexpr auto last_port = hana::back(types);
			using LastPort = typename decltype(last_port)::type;
			static_assert(LastPort::offset + LastPort::size <= MemoryBytes,
			              "last data port is mapped outside the device memory");

			static_assert(!details::ports_overlaps(types), "a data port overlaps");

			hana::for_each(types, [&](auto port) {
				using DataPort = typename decltype(port)::type;
				_ports.push_back(std::make_unique<mmap_data_port<DataPort>>(this));
			});
		}

	  private:
		gsl::span<uint8_t, MemoryBytes> _memory;
		std::vector<std::unique_ptr<bus::data_port>> _ports;
	};
}