#include "parse_args.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "scope.hpp"
#include "bytecode.hpp"
#include "error.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <functional>

int main(int argc, char* argv[])
{
	std::vector<std::string_view> args(argv, argv + argc);
	auto main_file = parse_args(args);

	Scope global_scope;
	bytecodes_t bytecodes;

	try {
		Lexer lexer(main_file);
		bytecodes = parse_stmts(lexer, global_scope);

		Interpreter interpreter(bytecodes);
	}
	catch (night::fatal_error const& e) {
		std::cout << e.what();
	}
	catch (...) {
		std::cout << "oops! we've come across and unexpected error!\n"
			"please submit an issue on github: https://github.com/DynamicSquid/night";
	}
}