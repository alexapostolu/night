#include "bytecode.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <string>

void bytecode_create_int(bytecodes_t& codes, int i, int64_t num)
{
	if (i == -1)
		i = codes.size();

	if (num >= 0)
	{
		uint64_t uint64 = num;

		if (uint64 <= std::numeric_limits<uint8_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::U_INT1);
		else if (uint64 <= std::numeric_limits<uint16_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::U_INT2);
		else if (uint64 <= std::numeric_limits<uint32_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::U_INT4);
		else if (uint64 <= std::numeric_limits<uint64_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::U_INT8);
		else {}

		do {
			codes.insert(std::begin(codes) + i, uint64 & 0xFF);
			++i;
		} while (uint64 >>= 8);
	}
	else
	{
		int64_t int64 = num;

		if (int64 <= std::numeric_limits<uint8_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::S_INT1);
		else if (int64 <= std::numeric_limits<uint16_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::S_INT2);
		else if (int64 <= std::numeric_limits<uint32_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::S_INT4);
		else if (int64 <= std::numeric_limits<uint64_t>::max())
			codes.insert(std::begin(codes) + i, (bytecode_t)BytecodeType::S_INT8);
		else {}

		do {
			codes.insert(std::begin(codes) + i, int64 & 0xFF);
			++i;
		} while (int64 >>= 8);
	}
}

void number_to_bytecode(bytecodes_t& codes, uint64_t num)
{
	if (num >= 0)
	{
		uint64_t uint64 = num;

		if (uint64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT1);
		else if (uint64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT2);
		else if (uint64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT4);
		else if (uint64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT8);
		else {}

		do {
			codes.push_back(uint64 & 0xFF);
		} while (uint64 >>= 8);
	}
	else
	{
		int64_t int64 = num;

		if (int64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT1);
		else if (int64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT2);
		else if (int64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT4);
		else if (int64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT8);
		else {}

		do {
			codes.push_back(int64 & 0xFF);
		} while (int64 >>= 8);
	}
}

void number_to_bytecode(bytecodes_t& codes, int64_t num)
{
	if (num >= 0)
	{
		uint64_t uint64 = num;

		if (uint64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT1);
		else if (uint64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT2);
		else if (uint64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT4);
		else if (uint64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT8);
		else {}

		do {
			codes.push_back(uint64 & 0xFF);
		} while (uint64 >>= 8);
	}
	else
	{
		int64_t int64 = num;

		if (int64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT1);
		else if (int64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT2);
		else if (int64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT4);
		else if (int64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT8);
		else {}

		do {
			codes.push_back(int64 & 0xFF);
		} while (int64 >>= 8);
	}
}

bytecodes_t number_to_bytecode(int64_t num)
{
	bytecodes_t codes;
	number_to_bytecode(codes, num);

	return codes;
}


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
	case BytecodeType::BOOL:
		return "BOOL";
	case BytecodeType::CHAR1:
		return "CHAR1";

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
		debug::unhandled_case((int)type);
		return "";
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