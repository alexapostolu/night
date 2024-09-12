#pragma once

#include <cassert>
#include <iterator>

namespace night {

template <typename ContainerDest, typename ContainerSrc>
void container_insert(ContainerDest& dest, ContainerSrc src, std::size_t position)
{
	assert(position <= dest.size());

	auto it = std::begin(dest);
	std::advance(it, position);

	dest.insert(it, std::begin(src), std::end(src));
}

}