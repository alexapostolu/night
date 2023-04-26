#include "value.hpp"

#include <stdexcept>
#include <variant>

std::string val_type_to_str(ValueType type)
{
	switch (type)
	{
	case ValueType::CHAR:
		return "CHAR";
	case ValueType::INT:
		return "INT";
	default:
		throw std::runtime_error("unhandled case");
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