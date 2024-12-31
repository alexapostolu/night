#pragma once

#include <stdint.h>

/**
 * @brief Data structures representing bytecodes and how they are stored.
 * 
 * Bytecode type refers to an action bytecode, while bytecode value refers to the
 * value(s) the action is being performed on. For example, "ADD" is a bytecode
 * type and "2" and "3" are bytecode values. Both are represented as 8 bit values
 * and are stored in the same data structure. The only way to differentiate
 * between the two when interpreting is solely through the language design.
 * 
 * **Design Decision #1**
 * bytecode_t is 8 bits.
 * 
 * My bytecode only has 63 possible values right now, and even if I did expand
 * it, I would have to expand it by over twice to reach the limit of 8 bits. So
 * 8 bits is plenty enough for Night.
 * 
 * **Design Decision #2**
 * bytecodes_t is a forward list and not a vector.
 * 
 * The main operations are appending/inserting other bytecode containers, and
 * list has a better time complexity for that operation than vector.
 */
typedef uint8_t byte_t;

/**
 * @brief Enumeration of all bytecode types in Night.
 */
enum {
	BytecodeType_S_INT1,
	BytecodeType_S_INT2,
	BytecodeType_S_INT4,
	BytecodeType_S_INT8,
	BytecodeType_U_INT1,
	BytecodeType_U_INT2,
	BytecodeType_U_INT4,
	BytecodeType_U_INT8,
	BytecodeType_FLOAT4,
	BytecodeType_FLOAT8,

	BytecodeType_NEGATIVE_I, BytecodeType_NEGATIVE_F,
	BytecodeType_NOT_I, BytecodeType_NOT_F,

	BytecodeType_ADD_I, BytecodeType_ADD_F, BytecodeType_ADD_S,
	BytecodeType_SUB_I, BytecodeType_SUB_F,
	BytecodeType_MULT_I, BytecodeType_MULT_F,
	BytecodeType_DIV_I, BytecodeType_DIV_F,
	BytecodeType_MOD_I,

	BytecodeType_LESSER_I, BytecodeType_LESSER_F, BytecodeType_LESSER_S,
	BytecodeType_GREATER_I, BytecodeType_GREATER_F, BytecodeType_GREATER_S,
	BytecodeType_LESSER_EQUALS_I, BytecodeType_LESSER_EQUALS_F, BytecodeType_LESSER_EQUALS_S,
	BytecodeType_GREATER_EQUALS_I, BytecodeType_GREATER_EQUALS_F, BytecodeType_GREATER_EQUALS_S,
	
	BytecodeType_EQUALS_I, BytecodeType_EQUALS_F, BytecodeType_EQUALS_S,
	BytecodeType_NOT_EQUALS_I, BytecodeType_NOT_EQUALS_F, BytecodeType_NOT_EQUALS_S,
	
	BytecodeType_AND,
	BytecodeType_OR,

	BytecodeType_INDEX_S,
	BytecodeType_INDEX_A,

	BytecodeType_I2F, BytecodeType_F2I,
	BytecodeType_F2B,

	BytecodeType_LOAD,
	BytecodeType_LOAD_ELEM,
	
	BytecodeType_STORE,
	BytecodeType_STORE_INDEX_A,
	BytecodeType_STORE_INDEX_S,
	
	BytecodeType_ALLOCATE_STR,
	BytecodeType_ALLOCATE_ARR,
	BytecodeType_ALLOCATE_ARR_AND_FILL,
	BytecodeType_FREE_STR,
	BytecodeType_FREE_ARR,

	BytecodeType_JUMP,
	BytecodeType_JUMP_N,
	BytecodeType_JUMP_IF_FALSE,

	BytecodeType_RETURN,
	BytecodeType_CALL
};

/*
 * Returns the corresponding bytecode as a string literal.
 */
char const* byte_to_str(byte_t byte);
