#pragma once

#include <string>

using value_t = int;

constexpr short primitive_count = 4;

enum class ValueType
{
	BOOL,
	CHAR,
	INT,
	FLOAT,
	STRING,
	ARRAY
};

bool compare_value_t(value_t type1, value_t type2);
bool is_object_t(value_t type);

namespace night
{

std::string to_str(value_t type, bool primitive = true);

}