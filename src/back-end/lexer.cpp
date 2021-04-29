#include "../../include/back-end/lexer.hpp"
#include "../../include/back-end/token.hpp"
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

void FindKeyword(const Location& loc, std::vector<Token>& tokens, std::string& token)
{
	if (token.empty())
		return;

	const std::unordered_map<std::string, TokenType> keywords{
		{ "true", TokenType::BOOL },
		{ "false", TokenType::BOOL },
		{ "set", TokenType::SET },
		{ "if", TokenType::IF },
		{ "else", TokenType::ELSE },
		{ "while", TokenType::WHILE },
		{ "for", TokenType::FOR },
		{ "def", TokenType::DEF },
		{ "return", TokenType::RETURN },
		{ "import", TokenType::IMPORT },
		{ "include", TokenType::IMPORT }
	};

	if (const auto find_keyword = keywords.find(token); find_keyword != keywords.end())
		tokens.push_back(Token{ loc, find_keyword->second, token });
	else if (std::regex_match(token, std::regex("[0-9]+")))
		tokens.push_back(Token{ loc, TokenType::INT, token });
	else if (std::regex_match(token, std::regex("([0-9]+)(\\.[0-9]+)?")))
		tokens.push_back(Token{ loc, TokenType::FLOAT, token });
	else if (std::regex_match(token, std::regex("[a-zA-Z_][a-zA-Z_0-9]*")))
		tokens.push_back(Token{ loc, TokenType::VAR, token });
	else
		throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "unknown token '" + token + "'");

	token = "";
}

Lexer lexer_create(const std::string& file_name, const bool main_file)
{
	std::fstream code_file(file_name);
	if (!code_file.is_open()) {
		throw NIGHT_PREPROCESS_ERROR(
			"file '" + file_name + "' could not be opened",
			main_file ? night::learn_run : night::learn_include);
	}

	Lexer lexer = {
		&code_file,
		{ file_name, 0 },
		"", 0,
		{
			{ '+', { { '=', TokenType::ASSIGN }, { '\0', TokenType::OPERATOR } } },
			{ '-', { { '=', TokenType::ASSIGN }, { '\0', TokenType::OPERATOR } } },
			{ '*', { { '=', TokenType::ASSIGN }, { '\0', TokenType::OPERATOR } } },
			{ '/', { { '=', TokenType::ASSIGN }, { '\0', TokenType::OPERATOR } } },
			{ '%', { { '=', TokenType::ASSIGN }, { '\0', TokenType::OPERATOR } } },

			{ '>', { { '=', TokenType::OPERATOR }, { '\0', TokenType::OPERATOR } } },
			{ '<', { { '=', TokenType::OPERATOR }, { '\0', TokenType::OPERATOR } } },

			{ '|', { { '|', TokenType::OPERATOR } } },
			{ '&', { { '&', TokenType::OPERATOR } } },
			{ '!', { { '=', TokenType::OPERATOR }, { '\0', TokenType::OPERATOR } } },

			{ '.', { { '\0', TokenType::OPERATOR } } },

			{ '=', { { '=', TokenType::OPERATOR }, { '\0', TokenType::ASSIGN } } },

			{ '(', { { '\0', TokenType::OPEN_BRACKET } } },
			{ ')', { { '\0', TokenType::CLOSE_BRACKET } } },
			{ '[', { { '\0', TokenType::OPEN_SQUARE } } },
			{ ']', { { '\0', TokenType::CLOSE_SQUARE } } },
			{ '{', { { '\0', TokenType::OPEN_CURLY } } },
			{ '}', { { '\0', TokenType::CLOSE_CURLY } } },

			{ ':', { { '\0', TokenType::COLON } } },
			{ ',', { { '\0', TokenType::COMMA } } }
		}
	};

	getline(*(lexer.code_file), lexer.code_line);

	return lexer;
}

Token lexer_eat(Lexer& lexer, const bool next_line)
{
	while (lexer.code_line[lexer.i] == ' ' || lexer.code_line[lexer.i] == '\t')
		++lexer.i;

	if (lexer.code_line[lexer.i] == '#')
		return Token{ {}, TokenType::EOL };

	// scan strings

	if (lexer.code_line[lexer.i] == '"')
	{
		++lexer.i;

		std::string str;
		while (lexer.i < lexer.code_line.length() && lexer.code_line[lexer.i] != '"')
		{
			const bool new_line = lexer_new_line(lexer);
			if (lexer.i == lexer.code_line.length() && !new_line)
			{	
				throw NIGHT_COMPILE_ERROR(
					"expected closing quotes for string '" + str + "'",
					"",
					night::learn_variables);
			}

			// to do:
			// check backslash quotes

			str += lexer.code_line[lexer.i];
			++lexer.i;
		}

		lexer.i++; 
		return Token{ lexer.loc, TokenType::STRING, str };;
	}

	// scan keywords

	if (std::isalpha(lexer.code_line[lexer.i]))
	{

	}

	// scan numbers

	if (std::isdigit(lexer.code_line[lexer.i]))
	{

	}

	// scan symbols

	if (auto symbol = lexer.symbols.find(lexer.code_line[lexer.i]);
		symbol != lexer.symbols.end())
	{
		for (auto const& match : symbol->second)
		{
			if (match.first != '\0' && lexer.i < lexer.code_line.length() - 1 &&
				lexer.code_line[lexer.i + 1] == match.first)
			{
				lexer.i += 2;
				return { lexer.loc, match.second,
					std::string(1, lexer.code_line[lexer.i]) + match.first };
			}
			else if (match.first == '\0')
			{
				++lexer.i;
				return { lexer.loc, match.second,
					std::string(1, lexer.code_line[lexer.i]) };
			}
		}
	}

	throw NIGHT_COMPILE_ERROR(
		std::string("unknown symbol '") + lexer.code_line[lexer.i] + "'",
		"",
		night::learn_learn,
		lexer.loc);
}

Token lexer_curr(Lexer& lexer, bool const next_line)
{
	if (next_line)
	{
		if (curr.type == TokenType::EOL)
		{
			Token token = 
		}
	}
	if (!next_line)
		return lexer.curr;
}
