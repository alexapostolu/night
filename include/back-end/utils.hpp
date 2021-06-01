#pragma once

#include "token.hpp"
#include "stmt.hpp"
#include "../error.hpp"

#include <vector>
#include <string>

struct Location
{
	std::string file;
	int line, col;
};

namespace night {

std::string tos(TokenType const& type);

} // namespace night