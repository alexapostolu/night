#include "bytecode.hpp"
#include "type.hpp"

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

CreateVariable::CreateVariable(ValueType _type, std::string const& _name)
	: Bytecode(BytecodeType::CREATE_VARIABLE), type(_type), name(_name)
{

}

PushConstant::PushConstant(std::variant<char, int> const& _val)
	: Bytecode(BytecodeType::PUSH_CONSTANT), val(_val)
{

}

Operation::Operation(OperationType _type)
	: Bytecode(BytecodeType::OPERATION), type(_type)
{

}