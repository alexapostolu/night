#include "parse_args.hpp"
#include "parser/statement_parser.hpp"
#include "code_gen.hpp"
#include "error.hpp"
#include "statement_scope.hpp"
#include "value.h" // Hanlde return Value for the program.
#include "interpreter.h"

#include <iostream>
#include <exception>

int main(int argc, char* argv[])
{
	auto main_file = parse_args(argc, argv);
	if (main_file.empty())
		return 0;

	bytes_t bytes;

	try {
		auto statements = parse_file(main_file);

		bytes = code_gen(statements);
	}
	catch (night::error const& e) {
		e.what();
	}
	catch (std::exception const& e) {
		std::cout << "oops! we have come across an unexpected error!\n\n"
				  << e.what() << "\n\n"
				  << "please submit an issue on github, https://github.com/alexapostolu/night\n";
	}

	byte_t* c_bytes = new byte_t[bytes.size()];
	std::copy(std::cbegin(bytes), std::cend(bytes), c_bytes);

	Value** variables = new Value*[StatementScope::max_var_id];

	function_t* functions = new function_t[StatementScope::functions.size()];
	for (std::size_t i = 0; i < StatementScope::functions.size(); ++i)
	{
		for (auto& pair : StatementScope::functions)
		{
			if (pair.second.id == i)
			{
				functions[i].param_count = pair.second.param_names.size();
				functions[i].param_ids = pair.second.param_ids.data();
				functions[i].bytes_count = pair.second.bytes.size();
				functions[i].bytes = new byte_t[bytes.size()];
				std::copy(std::cbegin(pair.second.bytes), std::cend(pair.second.bytes), functions[i].bytes);
				break;
			}
		}
	}

	Value* ret = interpret_bytecodes(c_bytes, bytes.size(), variables, functions);
	delete[] c_bytes;
	for (std::size_t i = 0; i < StatementScope::max_var_id; ++i)
	{
		if (variables[i])
			value_destroy(variables[i]);
	}
	delete[] variables;

	if (!ret)
		return 0;

	switch (ret->is) {
	case Value::Val_Int: return static_cast<int>(ret->as.i);
	case Value::Val_uInt: return static_cast<int>(ret->as.ui);
	case Value::Val_Dbl: return static_cast<int>(ret->as.d);
	default: return 0;
	}
}