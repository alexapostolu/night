#include "parse_args.hpp"
#include "parser/statement_parser.hpp"
#include "code_gen.hpp"
#include "error.hpp"
#include "interpreter.h"
#include "value.h" // Hanlde return Value for the program.

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
	std::size_t i = 0;
	for (auto const& byte : bytes)
		c_bytes[i++] = byte;

	Value ret = interpret_bytecodes(c_bytes, NULL);
	delete[] c_bytes;

	switch (ret.is) {
	case Value::Int: return ret.as.i;
	case Value::uInt: return ret.as.ui;
	case Value::Dbl: return static_cast<int>(ret.as.d);
	default: return 0;
	}
}