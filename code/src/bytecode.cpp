#include "bytecode.hpp"

#include <limits>
#include <string>

bytecodes_t int_to_bytecodes(uint64_t uint64, int size)
{
	bytecodes_t codes;
	int count = 0;

	if ((size == -1 || size == 1) && uint64 <= std::numeric_limits<uint8_t>::max())
	{
		codes.push_back(BytecodeType_S_INT1);
		count = 1;
	}
	else if ((size == -1 || size == 2) && uint64 <= std::numeric_limits<uint16_t>::max())
	{
		codes.push_back(BytecodeType_S_INT2);
		count = 2;
	}
	else if ((size == -1 || size == 4) && uint64 <= std::numeric_limits<uint32_t>::max())
	{
		codes.push_back(BytecodeType_S_INT4);
		count = 4;
	}
	else if ((size == -1 || size == 8) && uint64 <= std::numeric_limits<uint64_t>::max())
	{
		codes.push_back(BytecodeType_S_INT8);
		count = 8;
	}

	while (count--)
	{
		codes.push_back(uint64 & 0xFF);
		uint64 >>= 8;
	}

	return codes;
}

std::string night::to_str(bytecode_t type)
{
	switch (type)
	{
	case BytecodeType_S_INT1: return "S_INT1";
	case BytecodeType_S_INT2: return "S_INT2";
	case BytecodeType_S_INT4: return "S_INT4";
	case BytecodeType_S_INT8: return "S_INT8";
	case BytecodeType_U_INT1: return "U_INT1";
	case BytecodeType_U_INT2: return "U_INT2";
	case BytecodeType_U_INT4: return "U_INT4";
	case BytecodeType_U_INT8: return "U_INT8";
	case BytecodeType_FLOAT4: return "FLOAT4";
	case BytecodeType_FLOAT8: return "FLOAT8";

	case BytecodeType_NEGATIVE_I: return "NEGATIVE_I";
	case BytecodeType_NEGATIVE_F: return "NEGATIVE_F";
	case BytecodeType_NOT_I: return "NOT_I";
	case BytecodeType_NOT_F: return "NOT_F";

	case BytecodeType_ADD_I: return "ADD_I";
	case BytecodeType_ADD_F: return "ADD_F";
	case BytecodeType_ADD_S: return "ADD_S";
	case BytecodeType_SUB_I: return "SUB_I";
	case BytecodeType_SUB_F: return "SUB_F";
	case BytecodeType_MULT_I: return "MULT_I";
	case BytecodeType_MULT_F: return "MULT_F";
	case BytecodeType_DIV_I: return "DIV_I";
	case BytecodeType_DIV_F: return "DIV_F";
	case BytecodeType_MOD_I: return "MOD_I";

	case BytecodeType_LESSER_I: return "LESSER_I";
	case BytecodeType_LESSER_F: return "LESSER_F";
	case BytecodeType_LESSER_S: return "LESSER_S";
	case BytecodeType_GREATER_I: return "GREATER_I";
	case BytecodeType_GREATER_F: return "GREATER_F";
	case BytecodeType_GREATER_S: return "GREATER_S";
	case BytecodeType_LESSER_EQUALS_I: return "LESSER_EQUALS_I";
	case BytecodeType_LESSER_EQUALS_F: return "LESSER_EQUALS_F";
	case BytecodeType_LESSER_EQUALS_S: return "LESSER_EQUALS_S";
	case BytecodeType_GREATER_EQUALS_I: return "GREATER_EQUALS_I";
	case BytecodeType_GREATER_EQUALS_F: return "GREATER_EQUALS_F";
	case BytecodeType_GREATER_EQUALS_S: return "GREATER_EQUALS_S";

	case BytecodeType_EQUALS_I: return "EQUALS_I";
	case BytecodeType_EQUALS_F: return "EQUALS_F";
	case BytecodeType_EQUALS_S: return "EQUALS_S";
	case BytecodeType_NOT_EQUALS_I: return "NOT_EQUALS_I";
	case BytecodeType_NOT_EQUALS_F: return "NOT_EQUALS_F";
	case BytecodeType_NOT_EQUALS_S: return "NOT_EQUALS_S";

	case BytecodeType_AND: return "AND";
	case BytecodeType_OR: return "OR";

	case BytecodeType_INDEX_S: return "INDEX_S";
	case BytecodeType_INDEX_A: return "INDEX_A";

	case BytecodeType_I2F: return "I2F";
	case BytecodeType_F2I: return "F2I";
	case BytecodeType_F2B: return "F2B";

	case BytecodeType_LOAD: return "LOAD";
	case BytecodeType_LOAD_ELEM: return "LOAD_ELEM";

	case BytecodeType_STORE: return "STORE";
	case BytecodeType_STORE_INDEX_A: return "STORE_INDEX_A";
	case BytecodeType_STORE_INDEX_S: return "STORE_INDEX_S";

	case BytecodeType_ALLOCATE_STR: return "ALLOCATE_STR";
	case BytecodeType_ALLOCATE_ARR: return "ALLOCATE_ARR";
	case BytecodeType_ALLOCATE_ARR_AND_FILL: return "ALLOCATE_ARR_AND_FILL";
	case BytecodeType_FREE_STR: return "FREE_STR";
	case BytecodeType_FREE_ARR: return "FREE_ARR";

	case BytecodeType_JUMP: return "JUMP";
	case BytecodeType_JUMP_N: return "JUMP_N";
	case BytecodeType_JUMP_IF_FALSE: return "JUMP_IF_FALSE";

	case BytecodeType_RETURN: return "RETURN";
	case BytecodeType_CALL: return "CALL";

	default: return "UNKNOWN";
	}
}