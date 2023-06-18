#pragma once

#include <string>
#include <vector>
#include <stdint.h>

using bytecode_t = uint8_t;
using bytecodes_t = std::vector<bytecode_t>;

// comments indicate the corresponding integer values following it
enum struct BytecodeType : bytecode_t
{
	S_INT1,					// value
	S_INT2,					// value
	S_INT4,					// value
	S_INT8,					// value
	U_INT1,					// value
	U_INT2,					// value
	U_INT4,					// value
	U_INT8,					// value
	FLOAT4,					// value
	FLOAT8,					// value

	BOOL,					// value
	CHAR1,					// value

	VARIABLE,				// variable id

	NEGATIVE,
	NOT,					// value

	ADD,					// value
	SUB,					// value
	MULT,					// value
	DIV,					// value

	ASSIGN,

	IF,						// index of last if bytecode
	ELIF,					// index of last elif bytecode
	ELSE,					// index of last else bytecode

	WHILE,					// index of last while bytecode
	FOR,					// index of last for bytecode

	RETURN,					// return value
	FUNC_CALL				//
};

// for error messages, usually the real strategy here is something like, storing a side table that maps bytecode indices to locations in the source

std::string bytecode_to_str(BytecodeType type);
std::string bytecode_to_str(bytecode_t type);