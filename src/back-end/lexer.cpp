#include "../../include/back-end/lexer.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <regex>
#include <string>
#include <vector>
#include <map>
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

std::vector<Token> Lexer(const Location& loc, const std::string& file_line)
{
	const std::unordered_map<char, std::vector<std::pair<char, TokenType> > > symbols{
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
	};

	std::vector<Token> tokens;
	std::string token = "";
	char in_string = ' ';
	for (std::size_t a = 0; a < file_line.length(); ++a)
	{
		// scan strings

		if ((file_line[a] == '\'' || file_line[a] == '"') && in_string == ' ')
		{
			FindKeyword(loc, tokens, token);

			in_string = file_line[a];
			continue;
		}
		else if (file_line[a] != in_string && in_string != ' ')
		{
			token += file_line[a];
			continue;
		}
		else if (file_line[a] == in_string && in_string != ' ')
		{
			// check string for escape characters

			ReplaceEscape(token, "\\a", '\a');
			ReplaceEscape(token, "\\b", '\b');
			ReplaceEscape(token, "\\f", '\f');
			ReplaceEscape(token, "\\n", '\n');
			ReplaceEscape(token, "\\r", '\r');
			ReplaceEscape(token, "\\t", '\t');
			ReplaceEscape(token, "\\v", '\v');

			tokens.push_back(Token{ loc, TokenType::STRING, token });
			token = "";

			in_string = ' ';
			continue;
		}

		// ignore comments and whitespace

		if (file_line[a] == '#')
			break;

		if (file_line[a] == ' ' || file_line[a] == '\t')
		{
			FindKeyword(loc, tokens, token);
			continue;
		}

		// distinguish between dot operator and decimals

		if (file_line[a] == '.' && !token.empty() && token.back() - '0' >= 0 && token.back() - '0' <= 9)
		{
			token += file_line[a];
			continue;
		}

		// find symbols

		if (auto symbol = symbols.find(file_line[a]); symbol != symbols.end())
		{
			FindKeyword(loc, tokens, token);

			for (const auto& match : symbol->second)
			{
				if (match.first != '\0' && a < file_line.length() - 1 && file_line[a + 1] == match.first)
				{
					tokens.push_back(Token{ loc, match.second, std::string(1, file_line[a]) + match.first });

					a++;
					goto CONTINUE_NEXT;
				}
				else if (match.first == '\0')
				{
					tokens.push_back(Token{ loc, match.second, std::string(1, file_line[a]) });

					goto CONTINUE_NEXT;
				}
			}

			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, std::string("unknown symbol '") + file_line[a] + "'");
		}

		token += file_line[a];

		CONTINUE_NEXT:;
	}

	FindKeyword(loc, tokens, token);

	// allows curly brackets to be optional on single statements
	///*
	if (!tokens.empty() &&
		tokens[0].type != TokenType::IF && tokens[0].type != TokenType::ELSE &&
		tokens[0].type != TokenType::WHILE && tokens[0].type != TokenType::FOR &&
		tokens.back().type != TokenType::OPEN_CURLY && tokens[0].type != TokenType::DEF)
		tokens.push_back(Token{ loc, TokenType::EOL, "EOF" });
	//*/
	//if (!tokens.empty())
		//tokens.push_back(Token{ loc, TokenType::EOL, "EOL" });

	return tokens;
}