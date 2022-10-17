#include "parse_args.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "scope.hpp"
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

	Scope global_scope;
	bytecodes_t bytecodes;

	// catch fatal compile errors
	try {
		Lexer lexer(main_file);
		bytecodes = parse_stmts(lexer, global_scope);
	}
	catch (...) {

	}

	// catch fatal runtime errors
	try {
		Interpreter interpreter(bytecodes);
	}
	catch (...) {

	}
}