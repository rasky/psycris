#pragma once
#include "../bus/bus.hpp"
#include "../meta.hpp"
#include <cstdint>

namespace psycris::hw {

	template <uint32_t Offset, uint8_t Bytes = 4>
	struct data_reg {
		static_assert(Bytes == 1 || Bytes == 2 || Bytes == 4, "Unsupported port size");

		static constexpr uint32_t offset = Offset;
		static constexpr uint8_t size = Bytes;

		using int_type = std::conditional_t<Bytes == 4, uint32_t, std::conditional_t<Bytes == 2, uint16_t, uint8_t>>;
	};

	namespace details {
		constexpr auto ports_overlaps = [](auto ports) -> bool {
			uint32_t last_offset = 0;
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

	enum class rw_buffers {
		shared,
		distinct,
	};

	enum class buffer {
		any,
		r,
		w,
	};

	template <typename Derived, size_t MemoryBytes, rw_buffers RW = rw_buffers::shared>
	class mmap_device : public bus::device {
	  private:
		/**
		 * \brief a `bus::data_port` that forwards the `post_write` callback to the `Derived` subclass.
		 *
		 * \tparam DataPort is a specialization of `data_reg`; the `post_write`
		 * method of this specific class calls the `wcb()` method in the
		 * `Derived` one that accepts an instance of this type as the first
		 * argument. The `Derived` class is not required to define a method for
		 * every `DataPort`s
		 */
		template <typename DataPort>
		struct mmap_data_port : public bus::data_port {
			// silence the ubsan; we are casting a mmap_device* to Derived*
			// before the latter is constructed. Using now  such pointer is
			// obviously wrong, but we will use it later, in the post_write
			// method, when the Derived class is correctly built.
			__attribute__((no_sanitize("vptr"))) mmap_data_port(mmap_device* d)
			    : bus::data_port{DataPort::offset, DataPort::size}, _device{static_cast<Derived*>(d)} {}

			void post_write([[maybe_unused]] uint32_t new_value, [[maybe_unused]] uint32_t old_value) const override {
				auto has_write_callback = hana::is_valid(
				    [](auto&& port) -> decltype(std::declval<Derived>().wcb(port, 0, 0)) {});

				if constexpr (has_write_callback(DataPort{})) {
					_device->wcb(DataPort{}, new_value, old_value);
				}
			}

			Derived* _device;
		};

		static constexpr bool shared_buffers = RW == rw_buffers::shared;

	  public:
		static constexpr size_t addressable_size = MemoryBytes;

		static constexpr size_t size = addressable_size * (shared_buffers ? 1 : 2);

		template <typename... DataPorts>
		mmap_device(gsl::span<uint8_t, addressable_size> r, gsl::span<uint8_t, addressable_size> w, DataPorts...)
		    : _read_memory{r}, _write_memory{w} {
			if constexpr (sizeof...(DataPorts) > 0) {
				initialize_ports<DataPorts...>();
			}
		}

		template <typename... DataPorts>
		mmap_device(gsl::span<uint8_t, addressable_size> buffer, DataPorts... ports)
		    : mmap_device{buffer, buffer, ports...} {
			if constexpr (!shared_buffers) {
				static_assert(sizeof(Derived) < 1, "This device requires two distinct buffers for read/write memory");
			}
		}

	  private:
		template <typename, typename = void>
		struct has_device_name : std::false_type {};

		template <typename T>
		struct has_device_name<T, std::void_t<decltype(T::device_name)>> : std::true_type {};

	  public:
		char const* name() const override {
			if constexpr (has_device_name<Derived>::value) {
				return Derived::device_name;
			}
			return "unknown device";
		}

		gsl::span<uint8_t> readable_memory() const override { return _read_memory; }

		gsl::span<uint8_t> writable_memory() const override { return _write_memory; }

		size_t memory_size() const override {
			assert(readable_memory().size() == writable_memory().size());
			return readable_memory().size();
		}

		std::vector<bus::data_port const*> const& ports() override { return _ports_ptrs; }

	  protected:
		template <typename DataPort, buffer b = buffer::any>
		auto read() const {
			if constexpr (b == buffer::any && !shared_buffers) {
				static_assert(sizeof(Derived) < 1, "You must specify the buffer from which to read");
			}

			auto ptr = b == buffer::w ? _write_memory.data() : _read_memory.data();
			typename DataPort::int_type val;
			std::memcpy(&val, ptr + DataPort::offset, DataPort::size);
			return val;
		}

		template <typename DataPort, buffer b = buffer::any>
		void write(typename DataPort::int_type val) {
			if constexpr (b == buffer::any && !shared_buffers) {
				static_assert(sizeof(Derived) < 1, "You must specify the buffer to write to");
			}

			auto ptr = b == buffer::w ? _write_memory.data() : _read_memory.data();
			std::memcpy(ptr + DataPort::offset, &val, DataPort::size);
		}

	  private:
		template <typename... DataPorts>
		void initialize_ports() {
			constexpr auto get_offset = [](auto reg) {
				using T = typename decltype(reg)::type;
				return hana::int_c<T::offset>;
			};
			// a tuple of type_t sorted by the port offset
			constexpr auto types = hana::sort.by(hana::ordering(get_offset), hana::tuple_t<DataPorts...>);

			constexpr auto last_port = hana::back(types);
			using LastPort = typename decltype(last_port)::type;
			static_assert(LastPort::offset + LastPort::size <= MemoryBytes,
			              "last data port is mapped outside the device memory");

			static_assert(!details::ports_overlaps(types), "a data port overlaps");

			hana::for_each(types, [&](auto port_def) {
				using DataPort = typename decltype(port_def)::type;

				auto port = std::make_unique<mmap_data_port<DataPort>>(this);
				_ports_ptrs.push_back(port.get());
				_ports.push_back(std::move(port));
			});
		}

	  private:
		/**
		 * \brief the device memory (not owned by this class)
		 */
		gsl::span<uint8_t, MemoryBytes> _read_memory;
		gsl::span<uint8_t, MemoryBytes> _write_memory;

		/**
		 * \brief The device data ports (if any).
		 */
		std::vector<std::unique_ptr<bus::data_port>> _ports;

		/**
		 * \brief a copy of the `_ports` vector used to implement the `device`
		 * interface.
		 *
		 * The `ports()` method must return a const-ref to an already existent
		 * vector, but i don't want to ditch the unique pointers from the
		 * `_ports` member.
		 */
		std::vector<bus::data_port const*> _ports_ptrs;
	};
}