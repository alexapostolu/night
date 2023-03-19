#pragma once

#include "interpreter.hpp"
#include "value.hpp"

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <queue>

enum struct BytecodeType
{
	CONSTANT,
	VARIABLE,

	NOT,
	ADD,
	SUB,
	MULT,
	DIV,

	ASSIGN,
	ADD_ASSIGN,
	SUB_ASSIGN,
	MULT_ASSIGN,
	DIV_ASSIGN,

	IF,
	ELIF,
	ELSE,
	END_IF,

	RETURN
};

using op_type = BytecodeType;

struct Bytecode
{
	Bytecode(BytecodeType _type);
	std::string to_str() const;

	BytecodeType type;
	int val; // optional

	int line, col; // for error messages
};

using bytecodes_t = std::vector<Bytecode>;

std::string bytecode_to_str(BytecodeType type);