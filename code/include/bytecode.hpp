#pragma once

#include "interpreter.hpp"
#include "value.hpp"
#include "error.hpp"

#include <string>
#include <vector>

using bytecode_size = uint8_t;

// comments indicate the corresponding integer values following it
enum struct BytecodeType : bytecode_size
{
	S_INT1,
	S_INT2,
	S_INT4,
	S_INT8,
	U_INT1,
	U_INT2,
	U_INT4,
	U_INT8,
	FLOAT4,
	FLOAT8,

	VARIABLE,				// variable id

	NOT,					//
	ADD,					//
	SUB,					//
	MULT,					//
	DIV,					//

	BOOL_ASSIGN,			// variable index
	CHAR_ASSIGN,			// variable index
	INT_ASSIGN,				// variable index

	IF,						// index of last if bytecode
	ELIF,					// index of last elif bytecode
	ELSE,					// index of last else bytecode

	WHILE,					// index of last while bytecode
	FOR,					// index of lats for bytecode

	RETURN,					// return value
	FUNC_CALL				//
};

// for error messages, usually the real strategy here is something like, storing a side table that maps bytecode indices to locations in the source

using bytecodes_t = std::vector<bytecode_size>;

std::string bytecode_to_str(BytecodeType type);