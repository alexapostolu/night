#include "bytecode.hpp"
#include "type.hpp"

#include <stdexcept>
#include <variant>
#include <string>

Bytecode::Bytecode(BytecodeType _type)
	: type(_type)
{

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

Operation::Operation(OperationType _type)
	: Bytecode(BytecodeType::OPERATION), type(_type)
{

}

std::string Operation::to_str() const
{
	switch (type)
	{
	case OperationType::NOT:
		return "NOT";
	case OperationType::ADD:
		return "ADD";
	case OperationType::SUB:
		return "SUB";
	case OperationType::MULT:
		return "MULT";
	case OperationType::DIV:
		return "DIV";
	default:
		throw std::runtime_error("unhandled case");
	}
}