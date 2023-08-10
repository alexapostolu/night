#include "debug.hpp"
#include "parser_scope.hpp"
#include "bytecode.hpp"

#include <iostream>

void debug::log_codes(bytecodes_t const& codes)
{
	return;

	std::clog << "[printing bytecodes]\n";
	for (auto it = std::cbegin(codes); it != std::cend(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::BOOL:
			std::cout << "BOOL " << (int)(*(++it)) << '\n';
			break;
		case BytecodeType::CHAR1:
			std::clog << "CHAR " << (int)(*(++it)) << '\n';
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
			std::clog << night::to_str(*it) << ' ' << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::STR:
			(++it);
			break;

		case BytecodeType::NEGATIVE: std::clog << "NEGATIVE\n"; break;
		case BytecodeType::NOT:		 std::clog << "NOT\n"; break;

		case BytecodeType::ADD:  std::clog << "ADD\n"; break;
		case BytecodeType::SUB:  std::clog << "SUB\n"; break;
		case BytecodeType::MULT: std::clog << "MULT\n"; break;
		case BytecodeType::DIV:  std::clog << "DIV\n"; break;

		case BytecodeType::LOAD:
			std::clog << "LOAD " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::STORE:
			std::clog << "STORE " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::JUMP_IF_FALSE:
			std::clog << "JUMP_IF_FALSE " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::JUMP:
			std::clog << "JUMP " << (int)(*(++it)) << '\n';
			break;

		case BytecodeType::RETURN:
			std::clog << "RETURN\n" << (int)(*(++it)) << '\n';
			break;
		case BytecodeType::CALL: {
			int id = (int)(*(++it));
			for (auto const& [name, func] : ParserScope::funcs)
			{
				if (id == func.id)
					std::clog << "CALL " << name << " (" << id << ")\n";
			}

			break;
		}

		default:
			throw debug::unhandled_case(*it);
		}
	}

	std::clog << "[end of bytecodes]";
}