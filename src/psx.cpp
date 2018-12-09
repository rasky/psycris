#include "psx.hpp"

namespace {
	namespace hana = boost::hana;
	using namespace hana::literals;

	struct invalid {};

	template <typename T>
	auto to_type_t = invalid{};

	template <typename... T>
	auto to_type_t<std::tuple<T...>> = hana::tuple_t<T...>;

	////////

	struct slice {
		size_t start;
		size_t size;
	};

	constexpr auto build_layout_map() {
		using namespace psycris;

		auto parts = to_type_t<psx::board::layout>;

		size_t start = 0;
		auto io_map = hana::transform(parts, [&](auto component) {
			using T = typename decltype(component)::type;

			slice s{start, T::size};
			start += T::size;
			return s;
		});

		return io_map;
	}

	constexpr auto memory_layout = build_layout_map();

	template <int Part>
	gsl::span<uint8_t> v(gsl::span<uint8_t> m) {
		slice part_view = memory_layout[hana::int_c<Part>];
		return m.subspan(part_view.start, part_view.size);
	}
}

namespace psycris {
	psx::psx()
	    : _board_memory(psx::board::memory_size()), cpu(_bus), ram(v<0>(_board_memory)), rom(v<1>(_board_memory)) {

		_bus.connect({0x1fc0'0000, 0x1fc8'0000}, rom);
		_bus.connect({0x9fc0'0000, 0x9fc8'0000}, rom);
		_bus.connect({0xbfc0'0000, 0xbfc8'0000}, rom);

		_bus.connect({0x0000'0000, 0x0020'0000}, ram);
		_bus.connect({0x8000'0000, 0x8020'0000}, ram);
		_bus.connect({0xa000'0000, 0xa020'0000}, ram);
	}
}