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

/*
 * Converts any integral type into a sequence of bytes (bytecodes) using little
 * endian format. The sequence of bytes starts with the type of the integer in
 * Bytecode.
 */
template <typename T>
bytecodes_t int_to_bytes(T value)
{
	static_assert(std::is_integral<T>::value);

	bytecodes_t bytes;

	switch (sizeof T) {
		case 1: bytes.push_back(BytecodeType_S_INT1); break;
		case 2: bytes.push_back(BytecodeType_S_INT2); break;
		case 4: bytes.push_back(BytecodeType_S_INT4); break;
		case 8: bytes.push_back(BytecodeType_S_INT8); break;
		default: throw debug::unhandled_case(sizeof T);
	}

	for (size_t i = 0; i < sizeof T; ++i)
	{
		bytes.push_back(value & 0xFF);
		value >>= 8;
	}

	return bytes;
}