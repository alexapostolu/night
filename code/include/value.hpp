#pragma once

#include <string>
#include <variant>

enum class ValueType
{
	BOOL,
	CHAR,
	INT
};

struct Value
{
	ValueType type;
	std::variant<bool, char, int> val;
};

std::string val_type_to_str(ValueType type);
std::string val_to_str(ValueType type, std::variant<char, int> const& val);