#include "lexer.hpp"
#include "token.hpp"
#include "error.hpp"

#include <fstream>
#include <iostream>
#include <cctype>
#include <string>
#include <vector>
#include <unordered_map>

Lexer::Lexer()
	: loc({ "testing file", 1, 0 }) {}

Lexer::Lexer(std::string const& _file_name)
	: file(_file_name), loc({ _file_name, 1, 0 })
{
	if (!file.is_open())
		throw NIGHT_CREATE_FATAL_LEXER("file '" + loc.file + "' could not be found/opened");

	std::getline(file, file_line);
}

Lexer::~Lexer() {}

Token const& Lexer::eat()
{
	while (loc.col < file_line.size() && std::isspace(file_line[loc.col]))
		++loc.col;

	if (loc.col == file_line.size() || file_line[loc.col] == '#')
		return curr_tok = eat_new_line();


	if (std::isdigit(file_line[loc.col]))
		return curr_tok = eat_number();

	if (file_line[loc.col] == '"')
		return curr_tok = eat_string();

	if (std::isalpha(file_line[loc.col]) || file_line[loc.col] == '_')
		return curr_tok = eat_keyword();

	return curr_tok = eat_symbol();
}

Token const& Lexer::curr() const
{
	return curr_tok;
}

Token const& Lexer::expect(TokenType type, std::string const& err)
{
	eat();

	if (curr().type != type)
		throw NIGHT_CREATE_FATAL_LEXER("found '" + curr().str + "', expected " + tok_type_to_str(type) + " " + err);

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
		char ch;

		if (loc.col < file_line.length() - 1 && file_line[loc.col] == '\\')
		{
			match = true;

			switch (file_line[loc.col + 1])
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
			str += file_line[loc.col];
			++loc.col;
		}
		else
		{
			str += ch;
			loc.col += 2;
		}
	}

	++loc.col;
	return { TokenType::STR_LIT, str };
}

Token Lexer::eat_keyword()
{
	static std::unordered_map<std::string, TokenType> const keywords{
		{ "true", TokenType::BOOL_LIT },
		{ "false", TokenType::BOOL_LIT },
		{ "char8", TokenType::CHAR_TYPE },
		{ "int8", TokenType::INT_TYPE },
		{ "int16", TokenType::INT_TYPE },
		{ "int32", TokenType::INT_TYPE },
		{ "int64", TokenType::INT_TYPE },
		{ "uint8", TokenType::INT_TYPE },
		{ "uint16", TokenType::INT_TYPE },
		{ "uint32", TokenType::INT_TYPE },
		{ "uint64", TokenType::INT_TYPE },
		{ "if", TokenType::IF },
		{ "elif", TokenType::ELIF },
		{ "else", TokenType::ELSE },
		{ "for", TokenType::FOR },
		{ "while", TokenType::WHILE },
		{ "return", TokenType::RETURN }
	};

	std::string keyword;

	do {
		keyword += file_line[loc.col];
		++loc.col;
	} while (loc.col < file_line.length() && (std::isalpha(file_line[loc.col]) || std::isdigit(file_line[loc.col]) || file_line[loc.col] == '_'));

	if (auto it = keywords.find(keyword); it != keywords.end())
		return Token{ it->second, keyword };
	else
		return Token{ TokenType::VARIABLE, keyword };
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

		{ '>', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },
		{ '<', { { '=', TokenType::ASSIGN }, { '\0', TokenType::BINARY_OP } } },

		{ '|', { { '|', TokenType::BINARY_OP } } },
		{ '&', { { '&', TokenType::BINARY_OP } } },
		{ '!', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::UNARY_OP } } },

		{ '.', { { '.', TokenType::BINARY_OP }, { '\0', TokenType::BINARY_OP } } },

		{ '=', { { '=', TokenType::BINARY_OP }, { '\0', TokenType::ASSIGN } } },

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
		throw NIGHT_CREATE_FATAL_LEXER("unknown symbol '" + std::string(1, file_line[loc.col]) + "'");

	for (auto& [c, tok_type] : symbol->second)
	{
		if (c == '\0')
		{
			++loc.col;
			return { tok_type, std::string(1, file_line[loc.col - 1]) };
		}

		if (loc.col < file_line.length() - 1 && file_line[loc.col + 1] == c)
		{
			loc.col += 2;
			return { tok_type, std::string(1, file_line[loc.col - 2]) + std::string(1, c) };;
		}
	}

	throw NIGHT_CREATE_FATAL_LEXER("unknown symbol '" + file_line.substr(loc.col, 2) + "'");
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
		return { TokenType::END_OF_FILE, "end of file" };

	return eat();
}