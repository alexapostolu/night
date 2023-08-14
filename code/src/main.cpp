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
		/* parser */

		AST_Block ast_block = parse_file(main_file);

		/* code gen */

		bytecodes_t codes = code_gen(ast_block);

		// debugging
		debug::log_codes(codes);

		/* interpreter */

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