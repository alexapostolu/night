#pragma once

#include <memory>
#include <variant>
#include <string>
#include <vector>

enum struct BytecodeType
{
	CREATE_CONSTANT,
	CREATE_VARIABLE,
	PUSH_CONSTANT
};

struct Bytecode
{
	Bytecode(BytecodeType _type);

	BytecodeType type;
};

struct PushConstant : Bytecode
{
	PushConstant(std::variant<char, int> const& _val);

	std::variant<char, int> val;
};

enum struct CreateVariableType
{
	CHAR,
	INT
};

struct CreateVariable : Bytecode
{
	CreateVariable(CreateVariableType _type, std::string const& _name);

	CreateVariableType type;
	std::string name;
};

using bytecode_t  = std::shared_ptr<Bytecode>;
using bytecodes_t = std::vector<bytecode_t>;