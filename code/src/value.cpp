#include "value.hpp"
#include "bytecode.hpp"

#include <stdexcept>
#include <variant>

bool compare_value_t(value_t type1, value_t type2)
{
	return (type1 <= primitive_count && type2 <= primitive_count) ||
		   (type1 == type2);
}

bool is_object_t(value_t type)
{
	return type > primitive_count;
}

std::string val_type_to_str(value_t type)
{
	switch (type)
	{
	case (value_t)ValueType::BOOL:
		return "BOOL";
	case (value_t)ValueType::CHAR:
		return "CHAR";
	case (value_t)ValueType::S_INT:
		return "SIGNED_INT";
	case (value_t)ValueType::U_INT:
		return "UNSIGNED_INT";
	default:
		// return classes[type];
		return;
	}
}

std::string val_to_str(ValueType type, std::variant<char, int> const& val)
{
	switch (type)
	{
	case ValueType::CHAR:
		return std::string(std::get<char>(val), 1);
		break;
	case ValueType::INT:
		return std::to_string(std::get<int>(val));
		break;
	default:
		throw std::runtime_error("unhandled case");
	}
}

ValueType bytecode_type_to_val_type(BytecodeType type)
{
	return ValueType();
}
