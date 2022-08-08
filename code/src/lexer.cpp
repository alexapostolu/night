#include "lexer.hpp"
#include "token.hpp"

#include <fstream>
#include <iostream>
#include <cctype>
#include <string>

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

	if (i == file_line.size())
		return eat_new_line();

	if (file_line[i] == '#')
		return eat_new_line();

	// scan strings
	if (file_line[i] == '"')
	{
		++i;

		std::string str;
		while (true)
		{
			if (i == file_line.size() && !new_line())
				std::cout << "expected closing quotes for string '" + str + "'";

			if (file_line[i] == '"')
				break;

			if (i < file_line.length() - 1 && file_line[i] == '\\' && file_line[i + 1] == '"')
			{
				str += "\"";
				i += 2;
			}
			else
			{
				str += file_line[i];
				++i;
			}

		}
	}

	// scan keywords
	if (std::isalpha(code_line[i]))
	{
		std::string keyword;
		while (i < code_line.length() && (std::isalpha(code_line[i]) || code_line[i] == '_'))
		{
			keyword += code_line[i];
			++i;
		}

		auto it = keywords.find(keyword);
		curr = it != keywords.end()
			? Token{ loc, it->second,	  keyword }
		: Token{ loc, TokenType::VAR, keyword };

		return curr;
	}

	// scan numbers
	if (std::isdigit(code_line[i]))
	{
		std::string number;
		while (i < code_line.length() && std::isdigit(code_line[i]))
		{
			number += code_line[i];
			++i;
		}

		// scan decimal points
		if (i < code_line.length() - 1 && code_line[i] == '.' &&
			std::isdigit(code_line[i + 1]))
		{
			number += ".";

			++i;
			while (i < code_line.length() && std::isdigit(code_line[i]))
			{
				number += code_line[i];
				++i;
			}

			curr = { loc, TokenType::FLOAT_L, number };
			return curr;
		}

		curr = { loc, TokenType::INT_L, number };
		return curr;
	}

	// scan negative
	if (i < code_line.length() - 1 && code_line[i] == '-' && std::isdigit(code_line[i + 1]))
	{
		++i;

		curr = { loc, TokenType::UNARY_OP, "-" };
		return curr;
	}

	// scan symbols	
	if (auto symbol = symbols.find(code_line[i]); symbol != symbols.end())
	{
		for (auto& [c, tok_type] : symbol->second)
		{
			if (c == '\0')
			{
				std::string tok_data(1, code_line[i]);

				++i;

				curr = { loc, tok_type, tok_data };
				return curr;
			}
			if (i < code_line.length() - 1 && code_line[i + 1] == c)
			{
				std::string tok_data = std::string(1, code_line[i]) + c;

				i += 2;

				curr = { loc, tok_type, tok_data };
				return curr;
			}
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
