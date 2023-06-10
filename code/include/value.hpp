#pragma once

#include <string>
#include <variant>

enum class ValueType
{
	BOOL,
	CHAR,
	INT,
	OBJECT
};

struct Value
{
	ValueType type;
	int val; // stores index of class in object_array if Value type is object
};

std::string val_type_to_str(ValueType type);
std::string val_to_str(ValueType type, std::variant<char, int> const& val);
ValueType bytecode_type_to_val_type(BytecodeType type);