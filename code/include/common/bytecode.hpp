#pragma once

#include <list>
#include <string>
#include <type_traits>
#include <cstdint>

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
using bytecode_t = uint8_t;
using bytecodes_t = std::list<bytecode_t>;

/**
 * @brief Enumeration of all bytecode types in Night
 * 
 * The comments on some bytecode types represent how it is meant to be used, or
 * in other words, how it should appear on the Interpreter's stack. The comments
 * represent a stack growing from left to right.
 * 
 * In the comments, the term "numeric" represents an expansion of an integer,
 * float or a numeric expression. For example "numeric" could represent
 * "S_INT2 uint8 uint8" or "S_INT1 uint8 S_INT1 uint8 ADD".
 * 
 * **Design Decision #1**
 * BytecodeType is simply an "enum" and not an "enum class".
 * 
 * Stroustrup did state three problems with enums,
 *   https://www.stroustrup.com/C++11FAQ.html#enum
 *   1. Conventional enums implicitly convert to int, causing errors when
 *         someone does not want an enumeration to act as an integer.
 *   2. Conventional enums export their enumerators to the surrounding scope,
 *        causing name clashes.
 *   3. The underlying type of an enum cannot be specified, causing confusion,
 *        compatibility problems, and makes forward declaration impossible.
 * 
 * But these problems are irrelevant to "enum class BytecodeType",
 * 
 * #1 is irrelevant because BytecodeType is supposed to be implicitly
 * converted to bytecode_t by design. #2 is solved because BytecodeType
 * prefixes all types with BytecodeType_. And #3 is solved because
 * BytecodeType specifies the underlying type as bytecode_t (C++11 feature).
 * 
 * Now, not only does BytecodeType render the benefits of enum classes
 * unnecessary, but surpasses them by being shorter and looking neater,
 * @code
 *   bytes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE); // enum class
 *   bytes.push_back(BytecodeType_JUMP_IF_FALSE);              // enum
 * @endcode
 * 
 * **Design Decision #2**
 * There exists multiple integer sizes, not just INT8 and UINT8.
 * 
 * In the interpreter, all ints are either stores as an 8 bit signed or unsigned
 * int, however, we still differentiate them here in bytecode to reduce the total
 * number of bytecodes present.
 */
enum : bytecode_t {
	_ByteType_INVALID_ = 0,

	ByteType_sINT1,
	ByteType_sINT2,
	ByteType_sINT4,
	ByteType_sINT8,
	ByteType_uINT1,
	ByteType_uINT2,
	ByteType_uINT4,
	ByteType_uINT8,

	ByteType_FLT4,
	ByteType_FLT8,

	ByteType_NEG_I, ByteType_NEG_F,
	ByteType_NOT_I, ByteType_NOT_F,

	ByteType_ADD_I, ByteType_ADD_F, ByteType_ADD_S,
	ByteType_SUB_I, ByteType_SUB_F,
	ByteType_MUL_I, ByteType_MUL_F,
	ByteType_DIV_I, ByteType_DIV_F,
	ByteType_MOD,

	ByteType_LT_I, ByteType_LT_F, ByteType_LT_S,
	ByteType_LE_I, ByteType_LE_F, ByteType_LE_S,
	ByteType_GT_I, ByteType_GT_F, ByteType_GT_S,
	ByteType_GE_I, ByteType_GE_F, ByteType_GE_S,
	
	ByteType_EQ_I, ByteType_EQ_F, ByteType_EQ_S,
	ByteType_NE_I, ByteType_NE_F, ByteType_NE_S,
	
	BytecodeType_AND,	// numeric, numeric, AND
	BytecodeType_OR,	// numeric, numeric, OR

	BytecodeType_INDEX_S,	// numeric(string), INDEX_S
	BytecodeType_INDEX_A,	// numeric(array), INDEX_A

	ByteType_LOAD,
	BytecodeType_LOAD_ELEM,

	ByteType_DUP,
	
	ByteType_STORE,
	ByteType_STORE_INPLACE,
	BytecodeType_STORE_INDEX_A,
	BytecodeType_STORE_INDEX_S,
	
	BytecodeType_ALLOCATE_STR,
	BytecodeType_ALLOCATE_ARR,
	BytecodeType_ALLOCATE_ARR_AND_FILL,
	BytecodeType_FREE_STR,
	BytecodeType_FREE_ARR,

	BytecodeType_JUMP,
	ByteType_JUMP_N,
	BytecodeType_JUMP_IF_FALSE,

	BytecodeType_RETURN,
	BytecodeType_CALL
};

template <typename T>
requires std::is_same<T, int8_t>::value ||
			 std::is_same<T, uint8_t>::value ||
			 std::is_same<T, int16_t>::value ||
			 std::is_same<T, uint16_t>::value ||
			 std::is_same<T, int32_t>::value ||
			 std::is_same<T, uint32_t>::value ||
			 std::is_same<T, int64_t>::value ||
			 std::is_same<T, uint64_t>::value
bytecodes_t int_to_bytes(T i)
{
	bytecodes_t bytes;
	
	if (std::is_same<T, int8_t>::value)
		bytes.push_back(ByteType_sINT1);
	else if (std::is_same<T, uint8_t>::value)
		bytes.push_back(ByteType_uINT1);
	else if (std::is_same<T, int16_t>::value)
		bytes.push_back(ByteType_sINT2);
	else if (std::is_same<T, uint16_t>::value)
		bytes.push_back(ByteType_uINT2);
	else if (std::is_same<T, int32_t>::value)
		bytes.push_back(ByteType_sINT4);
	else if (std::is_same<T, uint32_t>::value)
		bytes.push_back(ByteType_uINT4);
	else if (std::is_same<T, int64_t>::value)
		bytes.push_back(ByteType_sINT8);
	else if (std::is_same<T, uint64_t>::value)
		bytes.push_back(ByteType_uINT8);
	
	for (int j = 0; j < sizeof(T); ++j)
	{
		bytes.push_back(i & 0xFF);
		i >>= 8;
	}

	return bytes;
}

namespace night {

/**
 * @brief Bytecode type to string. Used in error messages and debugging.
 */
std::string to_str(bytecode_t type);

}