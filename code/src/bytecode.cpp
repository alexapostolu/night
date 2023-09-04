#include "bytecode.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <string>

std::string night::to_str(BytecodeType type)
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

	case BytecodeType::NOT_I:
		return "NOT_I";
	case BytecodeType::NOT_F:
		return "NOT_F";
	case BytecodeType::ADD_I:
		return "ADD_I";
	case BytecodeType::ADD_F:
		return "ADD_F";
	case BytecodeType::ADD_S:
		return "ADD_S";
	case BytecodeType::SUB_I:
		return "SUB_I";
	case BytecodeType::SUB_F:
		return "SUB_F";
	case BytecodeType::MULT_I:
		return "MULT_I";
	case BytecodeType::MULT_F:
		return "MULT_F";
	case BytecodeType::DIV_I:
		return "DIV_I";
	case BytecodeType::DIV_F:
		return "DIV_F";

	case BytecodeType::LOAD:
		return "LOAD";
	case BytecodeType::STORE:
		return "STORE";

	case BytecodeType::JUMP:
		return "JUMP";
	case BytecodeType::JUMP_IF_FALSE:
		return "JUMP_IF_FALSE";

	case BytecodeType::RETURN:
		return "RETURN";
	case BytecodeType::CALL:
		return "FUNC_CALL";

	default:
		throw debug::unhandled_case((int)type);
	}
}

std::string night::to_str(bytecode_t val)
{
	try {
		return night::to_str((BytecodeType)val);
	}
	catch (std::runtime_error const&) {
		return std::to_string((int)val);
	}
}