#include "parse_args.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "scope.hpp"
#include "error.hpp"

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
		Lexer lexer(main_file);

		Scope global_scope;
		auto bytecodes = parse_stmts(lexer, global_scope);

		for (auto code : bytecodes)
			std::cout << code->to_str() << '\n';

		Interpreter interpreter(bytecodes);
	}
	catch (night::fatal_error const& e) {
		std::cout << e.what();
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