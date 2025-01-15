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

	byte_t* c_bytes = (byte_t*)malloc(bytes.size() * sizeof(byte_t));
	std::copy(std::cbegin(bytes), std::cend(bytes), c_bytes);

	Value** variables = (Value**)malloc(StatementScope::variable_id * sizeof(Value*));
	for (std::size_t i = 0; i < StatementScope::variable_id; ++i)
		variables[i] = NULL;

	function_t* functions = (function_t*)malloc(StatementScope::functions.size() * sizeof(function_t));
	assert(functions);
	for (std::size_t i = 0; i < StatementScope::functions.size(); ++i)
	{
		for (auto& pair : StatementScope::functions)
		{
			if (pair.second.id == i)
			{
				functions[i].param_count = pair.second.param_ids.size();
				functions[i].param_ids = (uint64_t*)malloc(sizeof(uint64_t) * pair.second.param_ids.size());
				assert(functions[i].param_ids);

				for (std::size_t j = 0; j < pair.second.param_ids.size(); ++j)
					functions[i].param_ids[j] = pair.second.param_ids[j];

				functions[i].bytes_count = pair.second.bytes.size();
				functions[i].bytes = (byte_t*)malloc(sizeof(byte_t) * bytes.size());
				assert(functions[i].bytes);
				std::size_t j = 0;
				for (auto it : pair.second.bytes)
					functions[i].bytes[j++] = it;
				//std::copy(std::begin(pair.second.bytes), std::end(pair.second.bytes), functions[i].bytes);
				break;
			}
		}
	}

	/*Value* ret = */interpret_bytecodes(c_bytes, bytes.size(), variables, functions);
	//free(c_bytes);
	//for (std::size_t i = 0; i < StatementScope::variable_id; ++i)
	{
		//if (variables[i])
		//	value_destroy(variables[i]);
	}
	//free(variables);

	/*if (!ret)
		return 0;

	switch (ret->is) {
	case Value::Val_Int: return static_cast<int>(ret->as.i);
	case Value::Val_Dbl: return static_cast<int>(ret->as.d);
	default: return 0;
	}*/
}