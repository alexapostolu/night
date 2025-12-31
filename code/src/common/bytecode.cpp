#include "common/bytecode.hpp"

#include <limits>
#include <string>

std::string night::to_str(bytecode_t type)
{
	switch (type)
	{
	case ByteType_sINT1: return "sINT1";
	case ByteType_sINT2: return "sINT2";
	case ByteType_sINT4: return "sINT4";
	case ByteType_sINT8: return "sINT8";
	case ByteType_uINT1: return "uINT1";
	case ByteType_uINT2: return "uINT2";
	case ByteType_uINT4: return "uINT4";
	case ByteType_uINT8: return "uINT8";
	case ByteType_FLT4: return "FLT4";
	case ByteType_FLT8: return "FLT8";

	case ByteType_NEG_I: return "NEG_I";
	case ByteType_NEG_F: return "NEG_F";
	case ByteType_NOT_I: return "NOT_I";
	case ByteType_NOT_F: return "NOT_F";

	case ByteType_ADD_I: return "ADD_I";
	case ByteType_ADD_F: return "ADD_F";
	case ByteType_ADD_S: return "ADD_S";
	case ByteType_SUB_I: return "SUB_I";
	case ByteType_SUB_F: return "SUB_F";
	case ByteType_MUL_I: return "MULT_I";
	case ByteType_MUL_F: return "MULT_F";
	case ByteType_DIV_I: return "DIV_I";
	case ByteType_DIV_F: return "DIV_F";
	case ByteType_MOD: return "MOD";

	case ByteType_LT_I: return "LT_I";
	case ByteType_LT_F: return "LT_F";
	case ByteType_LT_S: return "LT_S";
	case ByteType_LE_I: return "LE_I";
	case ByteType_LE_F: return "LE_F";
	case ByteType_LE_S: return "LE_S";
	case ByteType_GT_I: return "GT_I";
	case ByteType_GT_F: return "GT_F";
	case ByteType_GT_S: return "GT_S";
	case ByteType_GE_I: return "GE_I";
	case ByteType_GE_F: return "GE_F";
	case ByteType_GE_S: return "GE_S";

	case ByteType_EQ_I: return "EQ_I";
	case ByteType_EQ_F: return "EQ_F";
	case ByteType_EQ_S: return "EQ_S";
	case ByteType_NE_I: return "NE_I";
	case ByteType_NE_F: return "NE_F";
	case ByteType_NE_S: return "NE_S";

	case BytecodeType_AND: return "AND";
	case BytecodeType_OR: return "OR";

	case BytecodeType_INDEX_S: return "INDEX_S";
	case BytecodeType_INDEX_A: return "INDEX_A";

	case ByteType_LOAD: return "LOAD";
	case BytecodeType_LOAD_ELEM: return "LOAD_ELEM";

	case ByteType_STORE: return "STORE";
	case BytecodeType_STORE_INDEX_A: return "STORE_INDEX_A";
	case BytecodeType_STORE_INDEX_S: return "STORE_INDEX_S";

	case BytecodeType_ALLOCATE_STR: return "ALLOCATE_STR";
	case BytecodeType_ALLOCATE_ARR: return "ALLOCATE_ARR";
	case BytecodeType_ALLOCATE_ARR_AND_FILL: return "ALLOCATE_ARR_AND_FILL";
	case BytecodeType_FREE_STR: return "FREE_STR";
	case BytecodeType_FREE_ARR: return "FREE_ARR";

	case BytecodeType_JUMP: return "JUMP";
	case ByteType_JUMP_N: return "JUMP_N";
	case BytecodeType_JUMP_IF_FALSE: return "JUMP_IF_FALSE";

	case BytecodeType_RETURN: return "RETURN";
	case BytecodeType_CALL: return "CALL";

	default: return "UNKNOWN";
	}
}