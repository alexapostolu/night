#include "debug.hpp"
#include "bytecode.hpp"

#include <iostream>

void debug::log_codes(bytecodes_t const& codes)
{
	for (auto it = std::cbegin(codes); it != std::cend(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::BOOL:
			std::cout << "BOOL " << (int)(*(++it)) << '\n';
			break;
		case BytecodeType::CHAR1:
			std::cout << "CHAR " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::S_INT1:
		case BytecodeType::S_INT2:
		case BytecodeType::S_INT4:
		case BytecodeType::S_INT8:
		case BytecodeType::U_INT1:
		case BytecodeType::U_INT2:
		case BytecodeType::U_INT4:
		case BytecodeType::U_INT8:
		case BytecodeType::FLOAT4:
		case BytecodeType::FLOAT8:
			std::cout << night::to_str(*it) << ' ' << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::NEGATIVE: std::cout << "NEGATIVE\n"; break;
		case BytecodeType::NOT:		 std::cout << "NOT\n"; break;

		case BytecodeType::ADD:  std::cout << "ADD\n"; break;
		case BytecodeType::SUB:  std::cout << "SUB\n"; break;
		case BytecodeType::MULT: std::cout << "MULT\n"; break;
		case BytecodeType::DIV:  std::cout << "DIV\n"; break;

		case BytecodeType::LOAD:
			std::cout << "LOAD " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::STORE:
			std::cout << "STORE " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::JUMP_IF_FALSE:
			std::cout << "JUMP_IF_FALSE " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::JUMP:
			std::cout << "JUMP " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::RETURN:
			std::cout << "RETURN\n" << (int)(*(++it)) << '\n';
			break;
		case BytecodeType::CALL:
			break;

		default:
			debug::throw_unhandled_case(*it);
		}
	}
}