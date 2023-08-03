#pragma once

#include <string>

// should be the number of elements in ValueType
constexpr short primitive_count = 5;

enum class ValueType
{
	BOOL,
	CHAR,
	INT,
	FLOAT,
	STRING,
};

using value_t = int;

// returns:
//   true  if equal
//   false if not
bool compare_value_t(value_t type1, value_t type2);

bool is_object_t(value_t type);

namespace night
{

std::string to_str(value_t type, bool primitive = true);

}