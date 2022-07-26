#include "../version/version.hpp"

#include <iostream>
#include <vector>
#include <string>

int main()
{
	std::cout << "Hello Night!\n";

	std::cout << "night v"
			  << NIGHT_vMAJOR << '.'
			  << NIGHT_vMINOR << '.'
			  << NIGHT_vPATCH << '\n';
}