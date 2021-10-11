#include "../../include/back-end/lexer.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

Lexer::Lexer(std::string_view file_name, bool main_file)
	: code_file(file_name.data()), loc({ file_name.data(), 1 }), i(0)
{
	if (!code_file.is_open())
		throw NIGHT_PREPROCESS_ERROR("file '" + loc.file + "' could not be opened");

	getline(code_file, code_line);
}

Token Lexer::eat(bool go_to_next_line)
{
	if (!next_token(go_to_next_line))
	{
		curr = go_to_next_line ? Token::_EOF : Token::_EOL;
		return curr;
	}

	loc.col = i;

	// scan strings
	if (code_line[i] == '"')
	{
		++i;

		std::string str;
		while (code_line[i] != '"')
		{
			if (i == code_line.length() && !next_line()) {
				throw night::error(
					__FILE__, __LINE__, night::error_compile, loc,
					"expected closing quotes for string '" + str + "'", "");
			}

			// account for backslash quotes
			if (i < code_line.length() - 1 && code_line[i] == '\\' && code_line[i] == '"')
			{
				str += "\"";
				i += 2;
			}
			else
			{
				str += code_line[i];
				++i;
			}
		}

		++i;

		replace_escape_chars(str);

		curr = { loc, TokenType::STR_L, str };
		return curr;
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

	throw night::error(
		__FILE__, __LINE__, night::error_compile, loc,
		std::string("unknown symbol '") + code_line[i] + "'",
		code_line[i] == '"' ? "did you mean to use double quotations `\"`" : "");
}

Token Lexer::get_curr() const
{
	return curr;
}

Location Lexer::get_loc() const
{
	return loc;
}

bool Lexer::next_line()
{
	if (!std::getline(code_file, code_line))
		return false;

	i = 0;
	loc.line++;

	return true;
}

bool Lexer::next_token(bool go_to_next_line)
{
	while (i < code_line.length() && std::isspace(code_line[i]))
		++i;

	assert(i <= code_line.length());
	if (i == code_line.length() || code_line[i] == '#')
	{
		if (go_to_next_line)
			return next_line() ? next_token(true) : false;
		else
			return false;
	}

	return true;
}

void Lexer::replace_escape_chars(std::string& token) const
{
	static std::unordered_map<std::string, char> const esc_chars{
		{ "\\a", '\a' }, { "\\b", '\b' }, { "\\f", '\f' }, { "\\n", '\n' },
		{ "\\r", '\r' }, { "\\t", '\t' }, { "\\v", '\v' }, { "\\\\", '\\' },
		{ "\\'", '\'' }
	};

	std::size_t pos = token.find("\\");
	while (pos < token.size() - 1)
	{
		std::string esc{ '\\', token[pos + 1] };

		if (auto it = esc_chars.find(esc); it != esc_chars.end())
			token.replace(pos, 2, std::string(1, it->second));

		pos = token.find("\\", pos + 1);
	}
}

std::unordered_map<char, std::vector<std::pair<char, TokenType> > > const Lexer::symbols{
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
	{ "let", TokenType::LET },
	{ "if", TokenType::IF },
	{ "elif", TokenType::ELIF },
	{ "else", TokenType::ELSE },
	{ "while", TokenType::WHILE },
	{ "for", TokenType::FOR },
	{ "fn", TokenType::FN },
	{ "return", TokenType::RETURN }
};