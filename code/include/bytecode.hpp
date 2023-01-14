#pragma once

#include "type.hpp"

#include <memory>
#include <variant>
#include <string>
#include <vector>

enum struct BytecodeType
{
	CREATE_CONSTANT,
	CREATE_VARIABLE,
	PUSH_CONSTANT,
	OPERATION
};

struct Bytecode
{
	Bytecode(BytecodeType _type);

	BytecodeType type;
};

struct CreateConstant : Bytecode
{
	CreateConstant(ValueType _type, std::variant<char, int> const& _val);

	ValueType type;
	std::variant<char, int> val;
};

struct CreateVariable : Bytecode
{
	CreateVariable(ValueType _type, std::string const& _name);

	ValueType type;
	std::string name;
};

struct PushConstant : Bytecode
{
	PushConstant(std::variant<char, int> const& _val);

	std::variant<char, int> val;
};

enum struct OperationType
{
	NOT,
	ADD,
	SUB
};

struct Operation : Bytecode
{
	Operation(OperationType _type);

	OperationType type;
};

using bytecode_t  = std::shared_ptr<Bytecode>;
using bytecodes_t = std::vector<bytecode_t>;