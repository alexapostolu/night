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

bytecodes_t int_to_bytecodes(uint64_t uint64, int size)
{
	bytecodes_t codes;
	int count;

	if ((size == -1 || size == 1) && uint64 <= std::numeric_limits<uint8_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT1);
		count = 1;
	}
	else if ((size == -1 || size == 2) && uint64 <= std::numeric_limits<uint16_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT2);
		count = 2;
	}
	else if ((size == -1 || size == 4) && uint64 <= std::numeric_limits<uint32_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT4);
		count = 4;
	}
	else if ((size == -1 || size == 8) && uint64 <= std::numeric_limits<uint64_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT8);
		count = 8;
	}
	else
		throw debug::unhandled_case(uint64);

	while (count--)
	{
		codes.push_back(uint64 & 0xFF);
		uint64 >>= 8;
	}

	return codes;
}