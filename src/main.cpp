#include "../include/front-end/front-end.hpp"
#include "../include/error.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
	try {
		FrontEnd(argc, argv);
		return 0;
	}
	catch (const Error& e) {
		std::cout << e.what();
	}
	catch (const std::exception& e) {
		std::cout << Error::UnexpectedError(e);
	}

	return 1;
}