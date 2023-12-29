#pragma once

#include <limits>
#include <stdint.h>
#include <vector>
#include <string>

using bytecode_t = uint8_t;
constexpr bytecode_t bytecode_t_lim = std::numeric_limits<bytecode_t>::max();;

using bytecodes_t = std::vector<bytecode_t>;

// comments indicate how the bytecode should be used
// [] indicate a value popped off the stack
// () indicate the next bytecode value
enum struct BytecodeType : bytecode_t
{
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

	NEGATIVE_I, NEGATIVE_F,				// [val] NEGATIVE 
	NOT_I, NOT_F,					//

	ADD_I, ADD_F, ADD_S,
	SUB_I, SUB_F,
	MULT_I, MULT_F,
	DIV_I, DIV_F,
	MOD_I,

	LESSER_I, LESSER_F, LESSER_S,
	GREATER_I, GREATER_F, GREATER_S,
	LESSER_EQUALS_I, LESSER_EQUALS_F, LESSER_EQUALS_S,
	GREATER_EQUALS_I, GREATER_EQUALS_F, GREATER_EQUALS_S,
	EQUALS_I, EQUALS_F, EQUALS_S,
	NOT_EQUALS_I, NOT_EQUALS_F, NOT_EQUALS_S,
	AND,
	OR,

	INDEX_S,
	INDEX_A,

	// Primitive to float/int type conversions
	I2F, F2I,
	F2B,

	LOAD,					// LOAD  (var_id)
	LOAD_ELEM,				// [id: S_INT] [size: S_INT] [indices: S_INT ...]
	
	STORE,					// STORE (id)
	STORE_INDEX,			// [indicies, id]
	
	ALLOCATE_STR,			// [elements: Value ...] [size: S_INT]
	ALLOCATE_ARR,			//
	ALLOCATE_ARR_AND_FILL,
	FREE_STR,				// [id: S_INT]
	FREE_ARR,				//

	JUMP_IF_FALSE,			// [cond] JUMP_IF_FALSE (offset)	// jumps to next in conditional chain
	JUMP,					// JUMP (offset)					// jumps to end of conditional chain
	NJUMP,

	RETURN,					// [val] RETURN
	CALL					// [parameters as expressions] FUNC_CALL
};

namespace night
{

std::string to_str(BytecodeType type);
std::string to_str(bytecode_t type);

}

bytecodes_t int_to_bytecodes(uint64_t uint64);