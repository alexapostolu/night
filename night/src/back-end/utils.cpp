#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"

#include <string>

std::string night::tos(TokenType const& type)
{
	switch (type)
	{
	case TokenType::OPEN_BRACKET:
		return "opening bracket";
	case TokenType::CLOSE_BRACKET:
		return "closing bracket";
	case TokenType::OPEN_CURLY:
		return "opening curly bracket";
	case TokenType::CLOSE_CURLY:
		return "closing curly bracket";
	case TokenType::WHILE:
		return "'while' keyword";
	case TokenType::FOR:
		return "'for' keyword";
	case TokenType::VAR:
		return "variable name";
	}
}