#include "parse_args.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "parser_scope.hpp"
#include "code_gen.hpp"
#include "interpreter.hpp"
#include "error.hpp"

#include <iostream>
#include <exception>
#include <string>
#include <vector>

void debug_codes(bytecodes_t const& codes);

int main(int argc, char* argv[])
{
	std::vector<std::string_view> args(argv, argv + argc);
	auto main_file = parse_args(args);

	if (main_file.empty())
		return 0;

	try {
		/* lexer */

		Lexer lexer(main_file);

		/* parser */

		ParserScope global_scope;
		AST_Block ast_block;
		auto stmt = parse_stmts(lexer, global_scope);
		while (!stmt.empty())
		{
			ast_block.insert(std::end(ast_block), std::begin(stmt), std::end(stmt));
			stmt = parse_stmts(lexer, global_scope);
		}

		/* code gen */

		bytecodes_t codes = code_gen(ast_block);

		// debugging
		debug_codes(codes);

		/* interpreter */

		interpret_bytecodes(codes);
	}
	catch (night::error const& e) {
		std::cout << e.what() << '\n';
	}
	catch (std::exception const& e) {
		std::cout << "oops! we've come across and unexpected error!\n"
				  << "    " << e.what() << '\n'
				  << "please submit an issue on github: https://github.com/DynamicSquid/night";
	}
	catch (...) {
		std::cout << "oops! we've come across and unexpected error!\n"
				  << "please submit an issue on github: https://github.com/DynamicSquid/night";
	}
}

void debug_codes(bytecodes_t const& codes)
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
		case BytecodeType::NOT:	std::cout << "NOT\n"; break;

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
		case BytecodeType::FUNC_CALL:
			break;

		default:
			night::throw_unhandled_case(*it);
		}
	}
}