#pragma once

#include <iostream>
#include <algorithm>

namespace night {

template<
	typename Container,
	typename ValType>
	bool contains(
		Container const& container,
		ValType val)
{
	return std::find(container.begin(), container.end(), val) != container.end();
}

template<
	typename Container,
	typename ValType1,
	typename... ValType2>
	bool contains(
		Container const& container,
		ValType1 val1,
		ValType2... val2)
{
	return std::find(container.begin(), container.end(), val1) != container.end() ||
		contains(container, val2...);
}

} // namespace night