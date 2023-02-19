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
	return bytecode_to_str(type);
}

Constant::Constant(ValueType _type, std::variant<char, int> const& _val)
	: Bytecode(BytecodeType::CONSTANT), type(_type), val(_val)
{

}

std::string Constant::to_str() const
{
	return "CONSTANT " + val_type_to_str(type) + " " + val_to_str(type, val);
}

Variable::Variable(std::string const& _name)
	: Bytecode(BytecodeType::CONSTANT), name(_name)
{

}

std::string Variable::to_str() const
{
	return "VARIABLE " + name;
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

std::string bytecode_to_str(BytecodeType type)
{
	switch (type)
	{
	case BytecodeType::CONSTANT:
		return "CONSTANT";
	case BytecodeType::VARIABLE:
		return "VARIABLE";
	case BytecodeType::CREATE_VARIABLE:
		return "CREATE_VARIABLE";
	case BytecodeType::STORE_CONSTANT:
		return "STORE_CONSTANT";
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
	case BytecodeType::ADD_ASSIGN:
		return "ADD_ASSIGN";
	case BytecodeType::SUB_ASSIGN:
		return "SUB_ASSIGN";
	case BytecodeType::MULT_ASSIGN:
		return "MULT_ASSIGN";
	case BytecodeType::DIV_ASSIGN:
		return "DIV_ASSIGN";
	default:
		throw std::runtime_error("unhandled case in Bytecode::to_str");
	}
}