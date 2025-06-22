/*
 * This file provides the definitions for all types in Night. Used by the
 * Parser to conduct type checking.
 *
 * Types are split up into primitives and arrays. Primitives include booleans,
 * characters, integers and floats; and arrays are based off those types.
 *
 * Since the elements of each array must be of the same type, we can represent
 * any array type with a primitive type and the dimension count. The dimension
 * for primitive types is 0.
 *
 * Integer types may include explicit size specifications. In scenarios where
 * the integer size is not needed or can't be determined (eg. the literal '2'
 * could be any size), then the general type INT is used; and is equivalent to
 * any other integral type.
 */

#pragma once

#include <string>
#include <cstdint>

using dim_t = uint8_t;

enum class Primitive {
	BOOL,
	CHAR,

	INT,
	INT8,
	INT16,
	INT32,
	INT64,
	uINT8,
	uINT16,
	uINT32,
	uINT64,

	FLOAT
};

struct Type
{
	Primitive prim;
	dim_t dim;

	bool is_int() const;
	bool is_prim() const;
	bool is_str() const;
	bool is_arr() const;

	Type() = default;

	Type(std::string const& _prim_s, dim_t _dim = 0);
	Type(Primitive _prim, dim_t _dim = 0);
	Type(Type const& _other);

	bool operator==(Primitive _prim) const;
	bool operator==(Type const& _type) const;
};

namespace night {

std::string to_str(Type const& type);

} // night::
