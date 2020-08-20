<<<<<<< HEAD
#include <exception>
#include <iostream>

#include "ExtractCode.h"
#include "DataTypes/Error.h"

int main()
{
	try {
		ExtractCode();
	}
	catch (const Error& e) {
		std::cout << e.what() << '\n';
	}
	catch (const std::exception& e) {
		std::cout << Error::DevError(e) << '\n';
	}
=======
#include <exception>
#include <iostream>

#include "ExtractCode.h"
#include "DataTypes/Error.h"

int main()
{
	try {
		ExtractCode();
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
>>>>>>> test1
}