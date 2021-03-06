#pragma once

#include "../back-end/token.hpp"
#include <string>

// entry point - calls on the back end as well
void FrontEnd(
	int argc,
	char** argv
);

/*
void EvaluateCommands(
	char* argv[]
);
*/

std::vector<Token> OpenFile(
	const std::string& file
);