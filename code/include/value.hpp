#pragma once

#include <string>
#include <variant>

constexpr short primitive_count = 2;

namespace val
{

using value_t = int;

enum class ValueType
{
	BOOL,
	CHAR,
	S_INT,
	U_INT
};

struct Value
{
	// params:
	//   _data converted into int64_t
	Value(ValueType _type, int _data);

	// type = 1, 2, 3, then it's primative
	// type > 3, signals the type of the object
	value_t type;
	
	// stores index of class in object_array if Value type is object
	std::variant<int64_t, uint64_t> data;
};

}

// returns:
//   true  if equal
//   false if not
bool compare_value_t(val::value_t type1, val::value_t type2);

bool is_object_t(val::value_t type);


namespace night
{

std::string to_str(val::value_t type, bool primitive = true);

}