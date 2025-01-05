#pragma once

#include "bytecode.h"
#include "debug.hpp"

#include <list>
#include <iterator>
#include <assert.h>

namespace night {

template <typename ContainerDest, typename ContainerSrc>
void container_insert(ContainerDest& dest, ContainerSrc src, std::size_t position)
{
	assert(position <= dest.size());

	auto it = std::begin(dest);
	std::advance(it, position);

	dest.insert(it, std::begin(src), std::end(src));
}

} // night::

using bytes_t = std::list<byte_t>;

/*
 * Converts any integral type into a sequence of bytes (bytecodes) using little
 * endian format. The sequence of bytes starts with the type of the integer in
 * Bytecode.
 * 
 * For specifying a specific size for the value, which is useful in jump
 * statements, call the function with an explicit template type,
 * @code
 *   int_to_bytes(10);			 // automatically choosing size of 1 byte
 *   int_to_bytes<uint64_t>(10); // manually choosing size of 8 bytes
 * @endcode
 */
template <typename T>
bytes_t int_to_bytes(T value)
{
	static_assert(std::is_integral<T>::value);

	bytes_t bytes;

	switch (sizeof(T)) {
	case 1: bytes.push_back(BytecodeType_U_INT1); break;
	case 2: bytes.push_back(BytecodeType_U_INT2); break;
	case 4: bytes.push_back(BytecodeType_U_INT4); break;
	case 8: bytes.push_back(BytecodeType_U_INT8); break;
	default: throw debug::unhandled_case(sizeof(T));
	}

	for (std::size_t i = 0; i < sizeof(T); ++i)
	{
		bytes.push_back(value & 0xFF);
		value >>= 8;
	}

	return bytes;
}
