#include "bytecode.hpp"
#include "type.hpp"

#include <stdexcept>
#include <variant>
#include <string>

Bytecode::Bytecode(BytecodeType _type)
	: type(_type)
{

}

std::string Bytecode::to_str() const
{
	switch (type)
	{
	case BytecodeType::IF:
		return "IF";
	case BytecodeType::ELIF:
		return "ELIF";
	case BytecodeType::ELSE:
		return "ELSE";
	case BytecodeType::END_IF:
		return "END_IF";
	case BytecodeType::NOT:
		return "NOT";
	case BytecodeType::ADD:
		return "ADD";
	case BytecodeType::SUB:
		return "SUB";
	case BytecodeType::MULT:
		return "MULT";
	case BytecodeType::DIV:
		return "DIV";
	default:
		throw std::runtime_error("unhandled case");
	}
}

CreateConstant::CreateConstant(ValueType _type, std::variant<char, int> const& _val)
	: Bytecode(BytecodeType::CREATE_CONSTANT), type(_type), val(_val)
{

}

std::string CreateConstant::to_str() const
{
	return "CREATE_CONSTANT " + val_type_to_str(type) + " " + val_to_str(type, val);
}

CreateVariable::CreateVariable(ValueType _type, std::string const& _name)
	: Bytecode(BytecodeType::CREATE_VARIABLE), type(_type), name(_name)
{
	
}

std::string CreateVariable::to_str() const
{
	return "CREATE_VARIABLE " + val_type_to_str(type) + " " + name;
}

StoreConstant::StoreConstant(std::string const& _name)
	: Bytecode(BytecodeType::STORE_CONSTANT), name(_name)
{

}

std::string StoreConstant::to_str() const
{
	return "STORE_CONSTANT " + name;
}