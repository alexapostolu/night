#include "parse_args.hpp"
#include "parser/statement_parser.hpp"
#include "code_gen.hpp"
#include "interpreter.hpp"
#include "error.hpp"

#include <iostream>
#include <exception>

int main(int argc, char* argv[])
{
	auto main_file = parse_args(argc, argv);
	if (main_file.empty())
		return 0;

	try {
		auto statements = parse_file(main_file);

		auto bytecodes = code_gen(statements);

		InterpreterScope scope;
		interpret_bytecodes(scope, bytecodes);

		if (night::error::get().warning_flag)
			night::error::get().what(true);

		return 0;
	}
	catch (night::error& e) {
		e.what();

		if (night::error::get().warning_flag)
			night::error::get().what(true);

		return 1;
	}
	catch (std::exception const& e) {
		std::cout << "oops! we have come across an unexpected error!\n\n"
				  << e.what() << "\n\n"
				  << "please submit an issue on github, https://github.com/alexapostolu/night\n";

		return 1;
	}
}