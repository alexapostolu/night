#include "front-end/front-end.hpp"
#include "error.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
	try {
		FrontEnd(argc, argv);
		return 0;
	}
	catch (night::error const& e) {
		std::cout << e.what();
	}
	catch (std::exception const& e) {
		std::cout << std::string("Uh oh! We've come across an unexpected error:\n\n    ") + e.what() +
			"\n\nPlease submit an issue on the GitHub page:\nhttps://github.com/dynamicsquid/night\n";
	}

	return 1;
}
