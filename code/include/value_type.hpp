#pragma once

#include <string>

using value_t = int;

constexpr short primitive_count = 3;

struct ValueType
{
	enum _type {
		BOOL,
		CHAR,
		INT,
		FLOAT,
		STRING
	};

	ValueType() = default;
	ValueType(_type _type, bool _is_arr = false);
	ValueType(value_t _type, bool _is_arr = false);

	bool operator==(ValueType const& _vt) const;

	bool is_object() const;

	value_t type;
	bool is_arr;
};

// returns
//   true if both are primitive, or both are same object
bool compare_value_t(ValueType vt1, ValueType vt2);

namespace night
{

std::string to_str(value_t type, bool primitive = true);
std::string to_str(ValueType vt, bool primitive = true);

}