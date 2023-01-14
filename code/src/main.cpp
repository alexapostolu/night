#include "parse_args.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "scope.hpp"
#include "error.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
	std::vector<std::string_view> args(argv, argv + argc);
	auto main_file = parse_args(args);

	try {
		Lexer lexer(main_file);
		
		Scope global_scope;
		Interpreter interpreter(parse_stmts(lexer, global_scope));
	}
	catch (night::fatal_error const& e) {
		std::cout << e.what();
	}
	catch (...) {
		std::cout << "oops! we've come across and unexpected error!\n"
			"please submit an issue on github: https://github.com/DynamicSquid/night";
	}
}