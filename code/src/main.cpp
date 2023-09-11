#include "parse_args.hpp"
#include "parser.hpp"
#include "parser_scope.hpp"
#include "code_gen.hpp"
#include "interpreter.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <iostream>
#include <exception>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
	std::vector<std::string_view> args(argv, argv + argc);
	auto main_file = parse_args(args);

	if (main_file.empty())
		return 0;

	try {
		/* Parser */
		// Calls the Lexer to get tokens, and
		// then returns ASTs containing type and value information.
		AST_Block ast_block = parse_file(main_file);

		/* Bytecode Generation */
		// Each AST first correctness checks itself using its type information,
		// and then it generates bytecodes using its value information.
		bytecodes_t codes = code_gen(ast_block);

		// debugging
		debug::log_codes(codes);

		/* Interpreter */
		// Interprets the bytecodes.
		InterpreterScope scope;
		interpret_bytecodes(scope, codes);
	}
	catch (night::error const& e) {
		std::cout << e.what() << '\n';
	}
	catch (std::exception const& e) {
		std::cout << "oops! we've come across and unexpected error!\n\n"
				  << e.what() << "\n\n"
				  << "please submit an issue on github: https://github.com/DynamicSquid/night\n";
	}
}