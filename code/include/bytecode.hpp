#pragma once

#include <string>
#include <list>
#include <vector>
#include <stdint.h>
#include <limits>

using bytecode_t = uint8_t;
constexpr bytecode_t bytecode_t_lim = std::numeric_limits<bytecode_t>::max();;

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

	JUMP,
	LABEL,

	JUMP_IF_FALSE,
	END_IF,

	WHILE,					// index of last while bytecode
	FOR,					// index of last for bytecode

	RETURN,					// return value
	FUNC_CALL				//
};

void bytecode_create_int(bytecodes_t& codes,int64_t num, int i = -1);

void number_to_bytecode(bytecodes_t& codes, uint64_t num);
void number_to_bytecode(bytecodes_t& codes, int64_t num);

bytecodes_t const& number_to_bytecode(int64_t num);

// for error messages, usually the real strategy here is something like, storing a side table that maps bytecode indices to locations in the source

namespace night
{

std::string to_str(BytecodeType type);
std::string to_str(bytecode_t type);

}