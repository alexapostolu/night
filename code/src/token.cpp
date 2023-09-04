#include "token.hpp"
#include "debug.hpp"

#include <string>

std::string night::to_str(TokenType type)
{
	switch (type)
	{
	case TokenType::UNARY_OP: return "unary operator";
	case TokenType::BINARY_OP: return "binary operator";
	case TokenType::ASSIGN: return "assignment";
	case TokenType::OPEN_BRACKET: return "open bracket";
	case TokenType::CLOSE_BRACKET: return "close bracket";
	case TokenType::OPEN_SQUARE: return "open square";
	case TokenType::CLOSE_SQUARE: return "close square";
	case TokenType::OPEN_CURLY: return "open curly";
	case TokenType::CLOSE_CURLY: return "close curly";
	case TokenType::COLON: return "colon";
	case TokenType::SEMICOLON: return "semicolon";
	case TokenType::COMMA: return "comma";
	case TokenType::BOOL_LIT: return "boolean";
	case TokenType::CHAR_LIT: return "character";
	case TokenType::INT_LIT: return "integer";
	case TokenType::FLOAT_LIT: return "float";
	case TokenType::STRING_LIT: return "string";
	case TokenType::VARIABLE: return "variable";
	case TokenType::IF: return "if";
	case TokenType::ELIF: return "elif";
	case TokenType::ELSE: return "else";
	case TokenType::FOR:return "for";
	case TokenType::WHILE: return "while";
	case TokenType::RETURN: return "return";
	case TokenType::END_OF_FILE: return "end of file";
	default: throw debug::unhandled_case((int)type);
	}
}