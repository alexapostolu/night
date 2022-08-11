#include "lexer.hpp"
#include "token.hpp"

#include <fstream>
#include <iostream>
#include <cctype>
#include <string>
#include <vector>
#include <unordered_map>

Lexer::Lexer(std::string_view file_name)
	: file(file_name.data()), i(0)
{
	if (!file.is_open())
		std::cout << "file not open\n";

	std::getline(file, file_line);
}

Token Lexer::eat()
{
	while (i < file_line.size() && std::isspace(file_line[i]))
		++i;

	if (i == file_line.size() || file_line[i] == '#')
		return eat_new_line();


	if (std::isdigit(file_line[i]))
		return eat_number();

	if (file_line[i] == '"')
		return eat_string();

	if (std::isalpha(file_line[i]) || file_line[i] == '_')
		return eat_keyword();

	return eat_symbol();
}

Token Lexer::eat_string()
{
	++i;

	std::string str;

	while (true)
	{
		if (i == file_line.size() && !new_line())
			std::cout << "expected closing quotes for string '" + str + "'";

		if (file_line[i] == '"')
			break;

		bool match = false;
		char ch;

		if (i < file_line.length() - 1 && file_line[i] == '\\')
		{
			match = true;

			switch (file_line[i + 1])
			{
			case '\\': ch = '\\'; break;
			case '"':  ch = '\"'; break;
			case 'n':  ch = '\n'; break;
			case 't':  ch = '\t'; break;
			default:
				match = false;
			}
		}

		
		if (!match)
		{
			str += file_line[i];
			++i;
		}
		else
		{
			str += ch;
			i += 2;
		}
	}

	++i;
	return { TokenType::STR_LIT, str };
}

Token Lexer::eat_keyword()
{
	static std::unordered_map<std::string, TokenType> const keywords{
		{ "true", TokenType::BOOL_LIT },
		{ "false", TokenType::BOOL_LIT },
		{ "bool", TokenType::FOR },
		{ "char", TokenType::FOR },
		{ "int", TokenType::FOR },
		{ "str", TokenType::FOR },
		{ "if", TokenType::IF },
		{ "elif", TokenType::ELIF },
		{ "else", TokenType::ELSE },
		{ "for", TokenType::FOR },
		{ "while", TokenType::WHILE },
		{ "return", TokenType::RETURN }
	};

	std::string keyword;

	do {
		keyword += file_line[i];
		++i;
	} while (i < file_line.length() && (std::isalpha(file_line[i]) || file_line[i] == '_'));

	if (auto it = keywords.find(keyword); it != keywords.end())
		return Token{ it->second, keyword };
	else
		return Token{ TokenType::VARIABLE, keyword };
}

Token Lexer::eat_number()
{
	std::string number;

	do {
		number += file_line[i];
		++i;
	} while (i < file_line.length() && std::isdigit(file_line[i]));

	// floats
	if (file_line[i] == '.' && i < file_line.length() - 1 &&
		std::isdigit(file_line[i + 1]))
	{
		number += ".";
		++i;

		do {
			number += file_line[i];
			++i;
		} while (i < file_line.length() && std::isdigit(file_line[i]));

		return { TokenType::FLOAT_LIT, number };
	}

	return { TokenType::INT_LIT, number };
}

Token Lexer::eat_symbol()
{
	static std::unordered_map<char, std::vector<std::pair<char, TokenType> > > const symbols{
		{ '+', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
		{ '-', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
		{ '*', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
		{ '/', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
		{ '%', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },

		{ '>', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::BINARY_OP } } },
		{ '<', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::BINARY_OP } } },

		{ '|', { { '|', TokenType::BINARY_OP } } },
		{ '&', { { '&', TokenType::BINARY_OP } } },
		{ '!', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::UNARY_OP } } },

		{ '.', { { '.', TokenType::BINARY_OP }, { '\0', TokenType::BINARY_OP }}},

		{ '=', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::ASSIGN } } },

		{ '(', { { '\0', TokenType::OPEN_BRACKET } } },
		{ ')', { { '\0', TokenType::CLOSE_BRACKET } } },
		{ '[', { { '\0', TokenType::OPEN_SQUARE } } },
		{ ']', { { '\0', TokenType::CLOSE_SQUARE } } },
		{ '{', { { '\0', TokenType::OPEN_CURLY } } },
		{ '}', { { '\0', TokenType::CLOSE_CURLY } } },

		{ ':', { { '\0', TokenType::COLON } } },
		{ ',', { { '\0', TokenType::COMMA } } }
	};

	auto symbol = symbols.find(file_line[i]);
	if (symbol == symbols.end())
	{
		std::cout << "error\n";
	}

	for (auto& [c, tok_type] : symbol->second)
	{
		if (c == '\0')
		{
			++i;
			return { tok_type, std::string(1, file_line[i]) };
		}

		if (i < file_line.length() - 1 && file_line[i + 1] == c)
		{
			i += 2;
			return { tok_type, std::string(1, file_line[i]) + c };;
		}
	}
}

bool Lexer::new_line()
{
	i = 0;
	return (bool)std::getline(file, file_line);
}

Token Lexer::eat_new_line()
{
	if (!new_line())
		return { TokenType::END_OF_FILE, "end of file" };

	return eat();
}