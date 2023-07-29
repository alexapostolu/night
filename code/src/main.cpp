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
		auto ast_block = parse_stmts(lexer, global_scope);

		/* code gen */

		bytecodes_t codes = code_gen(ast_block);

		// debugging
		for (auto const& code : codes)
			std::cout << night::to_str(code) << '\n';

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