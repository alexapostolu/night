#include <exception>
#include <iostream>

#include "ExtractCode.hpp"
#include "DataTypes/Error.hpp"

int main(int argc, char* argv[] /*ask for console arguments*/)
{
	try {
		if (argc > 0) // if there is at least one console args
		{
			// extract the filename on the first console argument
			ExtractCode(argv[1]);
		}
	}
	catch (const Error& e) { //error handlings
		std::cout << e.what() << '\n';
	}
	catch (const std::exception& e) {
		std::cout << Error::DevError(e) << '\n';
	}
	catch (...) {
		std::cout << Error::DevError("unhandled exception") << '\n';
	}
}
