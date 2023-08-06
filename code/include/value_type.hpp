#pragma once

#include <string>

using value_t = int;

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

bool compare_value_t(value_t type1, value_t type2);
bool is_object_t(value_t type);

namespace night
{

std::string to_str(value_t type, bool primitive = true);

}