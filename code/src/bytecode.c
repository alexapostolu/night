#include "bytecode.h"

char const* byte_to_str(byte_t byte)
{
	switch (byte)
	{
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

	case ByteType_SUBSCRIPT_S: return "SUBSCRIPT_S";
	case ByteType_SUBSCRIPT_A: return "SUBSCRIPT_A";

	case BytecodeType_I2F: return "I2F";
	case BytecodeType_F2I: return "F2I";
	case BytecodeType_F2B: return "F2B";

	case BytecodeType_LOAD: return "LOAD";

	case BytecodeType_STORE: return "STORE";
	case ByteType_STORE_SUBSCRIPT_A: return "STORE_INDEX_A";
	case ByteType_STORE_SUBSCRIPT_S: return "STORE_INDEX_S";

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