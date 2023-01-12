#include "bytecode.hpp"

PushConstant::PushConstant(std::variant<char, int> const& _val)
	: Bytecode(BytecodeType::PUSH_CONSTANT), val(_val)
{

}

CreateVariable::CreateVariable(CreateVariableType _type, std::string const& _name)
	: Bytecode(BytecodeType::CREATE_VARIABLE), type(_type), name(_name)
{

}