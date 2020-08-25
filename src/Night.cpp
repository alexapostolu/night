#include <exception>
#include <iostream>

#include "ExtractCode.h"
#include "DataTypes/Error.h"

int main(int argc, char *argv[])
{
	try {
		if (argc > 0){
			ExtractCode(argv[1]);
		} else {
			char *sdefault = (char *)"source.night";
			ExtractCode(sdefault);
		}
	}
	catch (const Error& e) {
		std::cout << e.what() << '\n';
	}
	catch (const std::exception& e) {
		std::cout << Error::DevError(e) << '\n';
	}
	catch (...) {
		std::cout << Error::DevError("unhandled exception") << '\n';
	}
}