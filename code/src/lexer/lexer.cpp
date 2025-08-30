#include "lexer/lexer.hpp"
#include "common/token.hpp"
#include "common/error.hpp"

#include <fstream>
#include <iostream>
#include <cctype>
#include <string>
#include <vector>
#include <unordered_map>
#include <assert.h>

Lexer::Lexer(std::string const& _file_name)
	: file(_file_name), loc({ _file_name, 1, 0 }), prev_tok(std::nullopt)
{
	if (!file.is_open())
		throw night::error::get().create_fatal_error("file '" + loc.file + "' could not be found/opened", loc);

	std::getline(file, file_line);
	eat();
}

Lexer::~Lexer()
{
	file.close();
}

Token const& Lexer::eat()
{
	assert(curr().type != TokenType::END_OF_FILE);

	if (prev_tok.has_value())
	{
		prev_tok.reset();
		return curr_tok;
	}

	// Ignore whitespace.
	while (loc.col < file_line.size() && std::isspace(file_line[loc.col]))
		++loc.col;

	// Ignore comments.
	if (loc.col == file_line.size() || file_line[loc.col] == '#')
		return curr_tok = eat_new_line();

	if (std::isdigit(file_line[loc.col]))
		return curr_tok = eat_number();

	if (file_line[loc.col] == '"')
		return curr_tok = eat_string();

	if (file_line[loc.col] == '\'')
		return curr_tok = eat_character();

	if (std::isalpha(file_line[loc.col]) || file_line[loc.col] == '_')
		return curr_tok = eat_keyword();

	return curr_tok = eat_symbol();
}

Token Lexer::peek()
{
	if (prev_tok.has_value())
		return curr_tok;

	auto tmp_tok = curr_tok;
	auto eat_tok = eat();
	prev_tok = tmp_tok;

	return eat_tok;
}

Token const& Lexer::curr() const
{
	if (prev_tok.has_value())
		return *prev_tok;

	return curr_tok;
}

Token const& Lexer::expect(TokenType type, std::string const& err, std::source_location const& s_loc)
{
	if (!prev_tok.has_value())
		eat();

	if (curr_tok.type != type)
		throw night::error::get().create_fatal_error("found '" + curr().str + "', expected " + night::to_str(type) + " " + err, loc, s_loc);

	prev_tok.reset();
	return curr_tok;
}

Token const& Lexer::curr_is(TokenType type, std::string const& err_msg, std::source_location const& s_loc)
{
	if (curr().type != type)
		throw night::error::get().create_fatal_error(
			err_msg.empty()
				? "found '" + night::to_str(curr_tok.type) + "', expected '" + night::to_str(type) + "'"
				: err_msg,
			loc, s_loc);

	return curr();
}

void Lexer::scan_code(std::string const& code)
{
	loc.col = 0;
	file_line = code;
	eat();
}

Token Lexer::eat_string()
{
	++loc.col;

	std::string str;

	while (true)
	{
		if (loc.col == file_line.size() && !new_line())
			std::cout << "expected closing quotes for string '" + str + "'";

		if (file_line[loc.col] == '"')
			break;

		bool match = false;
		char chr;

		if (loc.col < file_line.length() - 1 && file_line[loc.col] == '\\')
		{
			match = true;

			switch (file_line[loc.col + 1])
			{
			case '\\': chr = '\\'; break;
			case 'n':  chr = '\n'; break;
			case 't':  chr = '\t'; break;
			case '"':  chr = '\"'; break;
			case '0':  chr = '\0'; break;
			default:
				match = false;
			}
		}

		
		if (!match)
		{
			str += file_line[loc.col];
			++loc.col;
		}
		else
		{
			str += chr;
			loc.col += 2;
		}
	}

	++loc.col;
	return { TokenType::STRING_LIT, str, loc };
}

Token Lexer::eat_character()
{
	++loc.col;

	if (loc.col == file_line.length())
		throw night::error::get().create_fatal_error("", loc);

	if (file_line[loc.col] == '\'')
		throw night::error::get().create_fatal_error("character can not not be empty", loc);

	char chr;

	if (file_line[loc.col] == '\\')
	{
		++loc.col;
		switch (file_line[loc.col])
		{
		case '\\': chr = '\\'; break;
		case 'n':  chr = '\n'; break;
		case 't':  chr = '\t'; break;
		case '"':  chr = '\"'; break;
		case '0':  chr = '\0'; break;
		default:
			throw night::error::get().create_fatal_error(std::string() + "unknown character '\\'" + file_line[loc.col], loc);
		}
	}
	else
	{
		chr = file_line[loc.col];
	}

	if (file_line[++loc.col] != '\'')
		throw night::error::get().create_fatal_error(std::string() + "found '" + file_line[loc.col] + "', expected closing quote at the end of character", loc);

	++loc.col;
	return { TokenType::CHAR_LIT, std::string(1, chr), loc };
}

Token Lexer::eat_keyword()
{
	static std::unordered_map<std::string, TokenType> const keywords{
		{ "true", TokenType::BOOL_LIT },
		{ "false", TokenType::BOOL_LIT },
		{ "char", TokenType::TYPE },
		{ "bool", TokenType::TYPE },
		{ "int8", TokenType::TYPE },
		{ "int16", TokenType::TYPE },
		{ "int32", TokenType::TYPE },
		{ "int64", TokenType::TYPE },
		{ "uint8", TokenType::TYPE },
		{ "uint16", TokenType::TYPE },
		{ "uint32", TokenType::TYPE },
		{ "uint64", TokenType::TYPE },
		{ "float", TokenType::TYPE },
		{ "if", TokenType::IF },
		{ "elif", TokenType::ELIF },
		{ "else", TokenType::ELSE },
		{ "for", TokenType::FOR },
		{ "while", TokenType::WHILE },
		{ "def", TokenType::DEF },
		{ "void", TokenType::VOID },
		{ "return", TokenType::RETURN }
	};

	std::string keyword;

	do {
		keyword += file_line[loc.col];
		++loc.col;
	} while (loc.col < file_line.length() && (std::isalpha(file_line[loc.col]) || std::isdigit(file_line[loc.col]) || file_line[loc.col] == '_'));

	if (auto it = keywords.find(keyword); it != keywords.end())
		return Token{ it->second, keyword, loc };
	else
		return Token{ TokenType::VARIABLE, keyword, loc };
}

Token Lexer::eat_number()
{
	std::string number;

	do {
		number += file_line[loc.col];
		++loc.col;
	} while (loc.col < file_line.length() && std::isdigit(file_line[loc.col]));

	// floats
	if (file_line[loc.col] == '.' && loc.col < file_line.length() - 1 &&
		std::isdigit(file_line[loc.col + 1]))
	{
		number += ".";
		++loc.col;

		do {
			number += file_line[loc.col];
			++loc.col;
		} while (loc.col < file_line.length() && std::isdigit(file_line[loc.col]));

		return { TokenType::FLOAT_LIT, number, loc };
	}

	return { TokenType::INT_LIT, number, loc };
}

Token Lexer::eat_symbol()
{
	static std::unordered_map<char, std::vector<std::pair<char, TokenType> > > const symbols{
		{ '+', { { '=', TokenType::ASSIGN_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },
		{ '-', { { '=', TokenType::ASSIGN_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },
		{ '*', { { '=', TokenType::ASSIGN_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },
		{ '/', { { '=', TokenType::ASSIGN_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },
		{ '%', { { '=', TokenType::ASSIGN_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },

		{ '>', { { '=', TokenType::BINARY_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },
		{ '<', { { '=', TokenType::BINARY_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },

		{ '|', { { '|', TokenType::BINARY_OPERATOR } } },
		{ '&', { { '&', TokenType::BINARY_OPERATOR } } },
		{ '!', { { '=', TokenType::BINARY_OPERATOR }, { '\0', TokenType::UNARY_OPERATOR } } },

		{ '.', { { '.', TokenType::BINARY_OPERATOR }, { '\0', TokenType::BINARY_OPERATOR } } },

		{ '=', { { '=', TokenType::BINARY_OPERATOR }, { '\0', TokenType::ASSIGN } } },

		{ '(', { { '\0', TokenType::OPEN_BRACKET } } },
		{ ')', { { '\0', TokenType::CLOSE_BRACKET } } },
		{ '[', { { '\0', TokenType::OPEN_SQUARE } } },
		{ ']', { { '\0', TokenType::CLOSE_SQUARE } } },
		{ '{', { { '\0', TokenType::OPEN_CURLY } } },
		{ '}', { { '\0', TokenType::CLOSE_CURLY } } },

		{ ',', { { '\0', TokenType::COMMA } } },
		{ ':', { { '\0', TokenType::COLON } } },
		{ ';', { { '\0', TokenType::SEMICOLON } } }
	};

	auto symbol = symbols.find(file_line[loc.col]);
	if (symbol == symbols.end())
		throw night::error::get().create_fatal_error("unknown symbol '" + std::string(1, file_line[loc.col]) + "'", loc);

	for (auto& [c, tok_type] : symbol->second)
	{
		if (c == '\0')
		{
			++loc.col;
			return { tok_type, std::string(1, file_line[loc.col - 1]), loc };
		}

		if (loc.col < file_line.length() - 1 && file_line[loc.col + 1] == c)
		{
			loc.col += 2;
			return { tok_type, std::string(1, file_line[loc.col - 2]) + std::string(1, c), loc };
		}
	}

	throw night::error::get().create_fatal_error("unknown symbol '" + file_line.substr(loc.col, 2) + "'", loc);
}

bool Lexer::new_line()
{
	++loc.line;
	loc.col = 0;

	return (bool)std::getline(file, file_line);
}

Token Lexer::eat_new_line()
{
	if (!new_line())
		return { TokenType::END_OF_FILE, "End of File", loc };

	return eat();
}