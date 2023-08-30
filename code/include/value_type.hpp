#pragma once

#include <optional>
#include <string>

constexpr short primitive_count = 3;

struct ValueType
{
	enum PrimType {
		BOOL,
		CHAR,
		INT,
		FLOAT,
		STR
	} type;

	// dimension of the array
	// elements of an array must be the same type, so for any n dimensional array,
	//  we just need one type, which is the type of the very bottom element
	int dim;

	ValueType() = default;
	ValueType(PrimType _type, int _dim = 0);
	bool operator==(PrimType _type) const;

	// is primitive (bool, char, int, float)
	bool is_prim() const;
};

// treats primitive types as the same
// used to differentiate print() overloads
bool compare_absolute_vt(ValueType const& vt1, ValueType const& vt2);

// treats primitive types as different
bool compare_relative_vt(ValueType const& vt1, ValueType const& vt2);

namespace night
{

std::string to_str(ValueType const& vt);

}