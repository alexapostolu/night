#include "../../include/back-end/token.hpp"

bool Token::is_value() const
{
	return type == TokenType::BOOL_L  || type == TokenType::INT_L ||
		   type == TokenType::FLOAT_L || type == TokenType::STR_L ||
		   type == TokenType::VAR     || type == TokenType::OPEN_SQUARE;
}

Token const Token::_EOL{ {}, TokenType::EOL, "EOL" };
Token const Token::_EOF{ {}, TokenType::_EOF, "EOF" };