#pragma once

#include "interpreter.hpp"
#include "value.hpp"
#include "error.hpp"

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <queue>

// comments indicate the corresponding integer value in the Bytecode
enum struct BytecodeType
{
	CONSTANT,				// value
	VARIABLE,				// variable id

	NOT,					//
	ADD,					//
	SUB,					//
	MULT,					//
	DIV,					//

	BOOL_ASSIGN,			// variable id
	CHAR_ASSIGN,			// variable id
	INT_ASSIGN,				// variable id

	ADD_ASSIGN,				// variable id
	SUB_ASSIGN,				// variable id
	MULT_ASSIGN,			// variable id
	DIV_ASSIGN,				// variable id

	IF,						// index of last if bytecode
	ELIF,					// index of last elif bytecode
	ELSE,					// index of last else bytecode

	FUNC,					// number of parameters
	RETURN,					// return value
	END_FUNC				//
};

using op_type = BytecodeType;

struct Bytecode
{
	std::string to_str() const;

	Location loc; // for error messages

	BytecodeType type;
	int val; // optional
};

using bytecodes_t = std::vector<Bytecode>;

std::string bytecode_to_str(BytecodeType type);