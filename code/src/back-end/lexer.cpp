#include "back-end/lexer.hpp"
#include "back-end/token.hpp"
#include "error.hpp"

#include <cctype>
#include <string>
#include <vector>
#include <unordered_map>

Lexer::Lexer(std::string_view file_name, const bool main_file [[maybe_unused]])
	: code_file(file_name.data()), loc({ file_name.data(), 1, 0 }), i(0)
{
	if (!code_file.is_open())
		throw NIGHT_PREPROCESS_ERROR("file '" + loc.file + "' could not be opened");

	getline(code_file, code_line);
}

Token Lexer::eat(const bool go_to_next_line)
{
	if (!next_token(go_to_next_line))
		return curr = go_to_next_line ? Token::_EOF : Token::_EOL;

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
			if (i < code_line.length() - 1 && code_line[i] == '\\' && code_line[i + 1] == '"')
			{
				str.push_back('\"');
				i += 2;
			}
			else str += code_line[i++];
		}

		++i;

		replace_escape_chars(str);

		return curr = { loc, TokenType::STR_L, str };
	}

	// scan keywords
	if (std::isalpha(code_line[i]))
	{
		std::string keyword;
		while (i < code_line.length() && (std::isalpha(code_line[i]) || code_line[i] == '_'))
			keyword += code_line[i++];

		auto it = keywords.find(keyword);
		return curr = it != keywords.end()
			? Token{ loc, it->second,	  keyword }
			: Token{ loc, TokenType::VAR, keyword };
	}

	// scan numbers
	if (std::isdigit(code_line[i]))
	{
		std::string number;
		while (i < code_line.length() && std::isdigit(code_line[i]))
			number += code_line[i++];

		// scan decimal points
		if (i < code_line.length() - 1 && code_line[i] == '.' &&
			std::isdigit(code_line[i + 1]))
		{
			number.push_back('.');

			++i;
			while (i < code_line.length() && std::isdigit(code_line[i]))
				number += code_line[i++];

			return curr = { loc, TokenType::FLOAT_L, number };
		}

		return curr = { loc, TokenType::INT_L, number };
	}

	// scan negative
	if (i < code_line.length() - 1 && code_line[i] == '-' && std::isdigit(code_line[i + 1]))
	{
		++i;

		return curr = { loc, TokenType::UNARY_OP, "-" };
	}

	// scan symbols	
	if (auto symbol = symbols.find(code_line[i]); symbol != symbols.end())
	{
		for (auto& [c, tok_type] : symbol->second)
		{
			if (!c) return curr = { loc, tok_type, std::string(1, code_line[i++]) };

			if (i < code_line.length() - 1 && code_line[i + 1] == c)
			{
				std::string tok_data({ code_line[i], c });

				i += 2;

				return curr = { loc, tok_type, tok_data };
			}
		}
	}

	throw night::error(
		__FILE__, __LINE__, night::error_compile, loc,
		std::string("unknown symbol '") + code_line[i] + "'",
		code_line[i] == '\'' ? "did you mean to use double quotations `\"`?" : "");
}

Token Lexer::peek(const bool go_to_next_line) noexcept
{
	auto const tmp_loc = loc;
	auto const tmp_code_ln = code_line;
	auto const tmp_i = i;
	auto const tmp_curr = curr;

	auto const next = eat(go_to_next_line);

	loc = tmp_loc,
	code_line = tmp_code_ln,
	i = tmp_i,
	curr = tmp_curr;

	return next;
}

Token Lexer::get_curr() const noexcept { return curr; }

Location Lexer::get_loc() const noexcept { return loc; }

bool Lexer::next_line() noexcept
{
	if (!std::getline(code_file, code_line))
		return false;

	i = 0;
	loc.line++;

	return true;
}

bool Lexer::next_token(const bool go_to_next_line) noexcept
{
	while (i < code_line.length() && std::isspace(code_line[i]))
		++i;

	assert(i <= code_line.length());
	if (i == code_line.length() || code_line[i] == '#')
	{
		return (go_to_next_line && next_line())
			? next_token(true)
			: false;
	}

	return true;
}

void Lexer::replace_escape_chars(std::string& token) const noexcept
{
	std::size_t pos = token.find('\\');
	while (pos < token.length() - 1)
	{
		char c = token[pos + 1];

		switch (c)
		{
#define CASE(x, s) case x: c = s; break
			CASE('a', '\a');
			CASE('b', '\b');
			CASE('f', '\f');
			CASE('n', '\n');
			CASE('r', '\r');
			CASE('t', '\t');
			CASE('v', '\v');
			case '\\':
			case '\'': token.erase(pos, 1); [[fallthrough]];
			default: c = 0; break;
		}
#undef CASE
		if (c) token.replace(pos, 2, 1, c);

		pos = token.find('\\', pos + 1);
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

std::unordered_map<std::string, TokenType> const Lexer::keywords{
	{ "true", TokenType::BOOL_L },
	{ "false", TokenType::BOOL_L },
	{ "let", TokenType::LET },
	{ "if", TokenType::IF },
	{ "elif", TokenType::ELIF },
	{ "else", TokenType::ELSE },
	{ "loop", TokenType::LOOP },
	{ "fn", TokenType::FN },
	{ "return", TokenType::RETURN }
};
