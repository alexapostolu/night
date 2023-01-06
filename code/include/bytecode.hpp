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
	BytecodeType type;
};

struct PushConstant : Bytecode
{
	std::variant<char, int> val;
};

enum struct CreateVariableType
{
	CHAR,
	INT
};

struct CreateVariable : Bytecode
{
	CreateVariableType type;
	std::string name;
};

using bytecode_t  = std::shared_ptr<Bytecode>;
using bytecodes_t = std::vector<bytecode_t>;