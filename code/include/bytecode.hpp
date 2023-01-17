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
	IF,
	ELIF,
	ELSE,
	END_IF,
	NOT,
	ADD,
	SUB,
	MULT,
	DIV
};

struct Bytecode
{
	Bytecode(BytecodeType _type);
	virtual std::string to_str() const;

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

using bytecode_t  = std::shared_ptr<Bytecode>;
using bytecodes_t = std::vector<bytecode_t>;