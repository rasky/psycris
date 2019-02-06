#pragma once
#include <boost/hana.hpp>

namespace psycris {
	namespace hana = boost::hana;

	struct empty {};

	template <typename T>
	auto to_type_t = empty{};

	template <typename... T>
	auto to_type_t<std::tuple<T...>> = hana::tuple_t<T...>;
}