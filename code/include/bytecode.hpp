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
	STORE_CONSTANT,
	OPERATION
};

struct Bytecode
{
	Bytecode(BytecodeType _type);
	virtual std::string to_str() const = 0;

	BytecodeType type;
};

struct CreateConstant : Bytecode
{
	CreateConstant(ValueType _type, std::variant<char, int> const& _val);
	std::string to_str() const override;

	ValueType type;
	std::variant<char, int> val;
};

struct CreateVariable : Bytecode
{
	CreateVariable(ValueType _type, std::string const& _name);
	std::string to_str() const override;

	ValueType type;
	std::string name;
};

struct StoreConstant : Bytecode
{
	StoreConstant(std::string const& _name);
	std::string to_str() const override;

	std::string name;
};

enum struct OperationType
{
	NOT,
	ADD,
	SUB,
	MULT,
	DIV
};

struct Operation : Bytecode
{
	Operation(OperationType _type);
	std::string to_str() const override;

	OperationType type;
};

using bytecode_t  = std::shared_ptr<Bytecode>;
using bytecodes_t = std::vector<bytecode_t>;