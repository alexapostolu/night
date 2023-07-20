#include "parse_args.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "parser_scope.hpp"
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

		/* codegen */

		bytecodes_t codes;
		for (auto const& ast : ast_block)
		{
			auto ast_codes = ast->generate_codes(global_scope);
			codes.insert(std::end(codes), std::begin(ast_codes), std::end(ast_codes));

			// debugging
			for (auto const& code : ast_codes)
				std::cout << night::to_str(code) << '\n';
			std::cout << '\n';
		}

		/* interpreter */

		Interpreter interpreter;
		interpret_bytecodes(interpreter, codes);
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