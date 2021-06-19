#include "../../include/back-end/lexer.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/error.hpp"

#include <regex>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <unordered_map>

void ReplaceEscape(std::string& token, const std::string& str, const char ch)
{
	std::size_t escape = token.find(str);
	while (escape != std::string::npos)
	{
		token[escape] = ch;
		token.erase(escape + 1, 1);

		escape = token.find(str, escape + 1);
	}
}



Lexer::Lexer(
	std::string const& file_name,
	bool const main_file)
	: code_file(file_name), loc({ file_name, 0 }), i(0)
{
	if (!code_file.is_open()) {
		throw NIGHT_PREPROCESS_ERROR(
			"file '" + file_name + "' could not be opened",
			main_file ? night::learn_run : night::learn_include);
	}

	getline(code_file, code_line);
}

Token Lexer::eat(
	bool const next_line)
{
	// go to next valid character
	while (true)
	{
		while (i < code_line.length() && code_line[i] == ' ' || code_line[i] == '\t')
			++i;

		if (i < code_line.length() && code_line[i] != '#')
			break;

		if (!next_line)
			return Token{ {}, TokenType::EOL };
	}

	// scan strings
	if (code_line[i] == '"')
	{
		++i;

		std::string str;
		while (true)
		{
			if (i == code_line.length())
			{
				if (!new_line()) {
					throw NIGHT_COMPILE_ERROR(
						"expected closing quotes for string '" + str + "'",
						"",
						night::learn_strings, loc);
				}
			}

			// to do:
			// check backslash quotes

			str += code_line[i];
			++i;
		}

		++i; 
		return Token{ loc, TokenType::STRING, str };;
	}

	// scan keywords
	if (std::isalpha(code_line[i]))
	{
		std::string keyword = code_line[i] + "";
		while (i < code_line.length() && std::isalpha(code_line[i]))
			keyword += code_line[i];

		if (auto const find_keyword = keywords.find(keyword);
			find_keyword != keywords.end())
			return Token{ loc, find_keyword->second, keyword };
		
		return Token{ loc, TokenType::VAR, keyword };
	}

	// scan numbers
	if (std::isdigit(code_line[i]))
	{
		std::string number = code_line[i] + "";
		while (i < code_line.length() && std::isalpha(code_line[i]))
			number += code_line[i];

		if (i < code_line.length() - 1 && code_line[i] == '.' &&
			std::isalpha(code_line[i + 1]))
		{
			++i;
			while (i < code_line.length() && std::isalpha(code_line[i]))
				number += code_line[i];

			return Token{ loc, TokenType::FLOAT_L, number };
		}

		return Token{ loc, TokenType::INT_L, number };
	}

	// scan negatives
	if (i < code_line.length() - 1 && code_line[i] == '-' && std::isdigit(code_line[i + 1]))
	{
		++i;
		return Token{ loc, TokenType::UNARY_OP, "-" };
	}

	// scan symbols	
	if (auto const symbol = symbols.find(code_line[i]); symbol != symbols.end()) {
		for (auto const& [c, tok_type] : symbol->second)
		{
			if (c == '\0')
			{
				++i;
				return { loc, tok_type, std::string(1, code_line[i]) };
			}
			if (c != '\0' && i < code_line.length() - 1 && code_line[i + 1] == c)
			{
				i += 2;
				return { loc, tok_type, std::string(1, code_line[i]) + c };
			}
		}
	}

	throw NIGHT_COMPILE_ERROR(
		loc,
		std::string("unknown symbol '") + code_line[i] + "'",
		"",
		night::learn_learn);
}

void Lexer::eat(
	TokenType   const& expect_type,
	bool	    const  next_line,
	std::string const& stmt_format,
	std::string const& stmt_learn)
{
	Token const curr = get_curr(false);
	Token const next = eat(next_line);
	if (next.type != expect_type) {
		throw NIGHT_COMPILE_ERROR(
			get_loc(),
			"expected " + night::tos(expect_type) + " after " + night::tos(curr.type),
			stmt_format, stmt_learn);
	}
}

Token Lexer::peek(bool const next_line)
{
	return Token();
}

Token Lexer::get_curr(
	bool const next_line)
{
	if (next_line)
	{
		while (curr.type == TokenType::EOL)
			curr = eat(true);
	}
	
	return curr;
}

Location Lexer::get_loc() const
{
	return loc;
}

bool Lexer::new_line()
{
	if (!std::getline(code_file, code_line))
		return false;

	i = 0;
	return true;
}

std::unordered_map<char, std::map<char, TokenType> > const Lexer::symbols{
	{ '+', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
	{ '-', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
	{ '*', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
	{ '/', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
	{ '%', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },

	{ '>', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::BINARY_OP } } },
	{ '<', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::BINARY_OP } } },

	{ '|', { { '|', TokenType::BINARY_OP } } },
	{ '&', { { '&', TokenType::BINARY_OP } } },
	{ '!', { { '=', TokenType::UNARY_OP }, { '\0', TokenType::BINARY_OP } } },

	{ '.', { { '\0', TokenType::BINARY_OP } } },

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

std::unordered_map<std::string, TokenType> const Lexer::keywords{
	{ "true", TokenType::BOOL_L },
	{ "false", TokenType::BOOL_L },
	{ "set", TokenType::SET },
	{ "if", TokenType::IF },
	{ "else", TokenType::ELSE },
	{ "switch", TokenType::SWITCH },
	{ "while", TokenType::WHILE },
	{ "for", TokenType::FOR },
	{ "fn", TokenType::FN },
	{ "return", TokenType::RETURN },
	{ "import", TokenType::IMPORT },
	{ "include", TokenType::IMPORT }
};