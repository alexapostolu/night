#include "parse_args.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "bytecode.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

/*
struct Stmt
{
	std::variant stmt;
}

struct File
{
	std:::vector<std::string> deps;
	std::string_view file_name;
	bool is_interpreted;
	std::vector<Stmt> stms;
}
*/

int main(int argc, char* argv[])
{
	std::vector<std::string_view> args(argv, argv + argc);
	auto main_file = parse_args(args);

	bytecodes_t bytecodes;

	// catch fatal compile errors
	try {
		Lexer lexer(main_file);
		Parser parser(lexer, bytecodes);
	}
	catch (...) {

	}

	// catch fatal runtime errors
	try {
		Interpreter interpreter(main_file);
	}
	catch (...) {

	}
}