#pragma once

#include <list>
#include <vector>
#include <string>
#include <limits>
#include <stdint.h>

using bytecode_t = uint8_t;
constexpr bytecode_t bytecode_t_lim = std::numeric_limits<bytecode_t>::max();;

using bytecodes_t = std::vector<bytecode_t>;

// comments indicate how the bytecode should be used
// [] indicate a value popped off the stack
// () indicate the next bytecode value
enum struct BytecodeType : bytecode_t
{
	BOOL,					// BOOL (val)
	CHAR1,					//

	S_INT1,					// S_INT1 (val)
	S_INT2,					// S_INT2 (val) (val)
	S_INT4,					//
	S_INT8,					//
	U_INT1,					//
	U_INT2,					//
	U_INT4,					//
	U_INT8,					//
	FLOAT4,					//
	FLOAT8,					//
	STR,					// S_INT1 (length) (characters)

	NEGATIVE,				// [val] NEGATIVE 
	NOT,					//

	ADD,					// [val] [val] ADD
	SUB,					//
	MULT,					//
	DIV,					//

	LESSER,
	GREATER,

	LOAD,					// LOAD  (var_id)
	STORE,					// STORE (var_id)

	JUMP_IF_FALSE,			// [cond] JUMP_IF_FALSE (offset)	// jumps to next in conditional chain
	JUMP,					// JUMP (offset)					// jumps to end of conditional chain
	NJUMP,

	RETURN,					// [val] RETURN
	CALL					// [parameters as variable initializations] FUNC_CALL
};

//void bytecode_create_int(bytecodes_t& codes, int64_t num, int i = -1);

void number_to_bytecode(bytecodes_t& codes, uint64_t num);
void number_to_bytecode(bytecodes_t& codes, int64_t num);

bytecodes_t number_to_bytecode(int64_t num);

namespace night
{

std::string to_str(BytecodeType type);
std::string to_str(bytecode_t type);

}