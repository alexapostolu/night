#include "bytecode.hpp"
#include "error.hpp"

#include <string>

std::string bytecode_to_str(BytecodeType type)
{
	switch (type)
	{
	case BytecodeType::S_INT1:
		return "S_INT1";
	case BytecodeType::S_INT2:
		return "S_INT2";
	case BytecodeType::S_INT4:
		return "S_INT4";
	case BytecodeType::S_INT8:
		return "S_INT8";
	case BytecodeType::U_INT1:
		return "U_INT1";
	case BytecodeType::U_INT2:
		return "U_INT2";
	case BytecodeType::U_INT4:
		return "U_INT4";
	case BytecodeType::U_INT8:
		return "U_INT8";
	case BytecodeType::FLOAT4:
		return "FLOAT4";
	case BytecodeType::FLOAT8:
		return "FLOAT8";
	case BytecodeType::BOOL:
		return "BOOL";
	case BytecodeType::CHAR1:
		return "CHAR1";

	case BytecodeType::VARIABLE:
		return "VARIABLE";

	case BytecodeType::NOT:
		return "NOT";
	case BytecodeType::ADD:
		return "ADD";
	case BytecodeType::SUB:
		return "SUB";
	case BytecodeType::MULT:
		return "MULT";
	case BytecodeType::DIV:
		return "DIV";

	case BytecodeType::ASSIGN:
		return "ASSIGN";

	case BytecodeType::IF:
		return "IF";
	case BytecodeType::ELIF:
		return "ELIF";
	case BytecodeType::ELSE:
		return "ELSE";

	case BytecodeType::WHILE:
		return "WHILE";
	case BytecodeType::FOR:
		return "FOR";

	case BytecodeType::RETURN:
		return "RETURN";
	case BytecodeType::FUNC_CALL:
		return "FUNC_CALL";

	default:
		throw night::unhandled_case((int)type);
	}
}

std::string bytecode_to_str(bytecode_t val)
{
	return std::to_string(val);
}