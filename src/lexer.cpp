#include "../include/lexer.h"
#include "../include/token.h"
#include "../include/error.h"

#include <regex>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

void FindKeyword(const std::string& file, const int line, std::vector<Token>& tokens, std::string& token)
{
	const std::unordered_map<std::string, TokenType> keywords{
		{ "true", TokenType::BOOL_VAL },
		{ "false", TokenType::BOOL_VAL },
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

	if (token.empty())
		return;

	const auto findKeyword = keywords.find(token);
	if (findKeyword != keywords.end())
		tokens.push_back(Token{ file, line, findKeyword->second, token });
	else if (std::regex_match(token, std::regex("((\\+|-)?([0-9]+)(\\.[0-9]+)?)|((\\+|-)?\\.?[0-9]+)")))
		tokens.push_back(Token{ file, line, TokenType::NUM_VAL, token });
	else if (std::regex_match(token, std::regex("[a-zA-Z_][a-zA-Z_0-9]*")))
		tokens.push_back(Token{ file, line, TokenType::VARIABLE, token });
	else
		throw Error(file, line, "unknown token '" + token + "'");

	token = "";
}

std::vector<Token> Lexer(const std::string& file, const int line, const std::string& fileLine)
{
	const std::unordered_map<char, std::vector<std::pair<char, TokenType> > > symbols{
		{ '+', { { '=', TokenType::ASSIGNMENT }, { '\0', TokenType::OPERATOR } } },
		{ '-', { { '=', TokenType::ASSIGNMENT }, { '\0', TokenType::OPERATOR } } },
		{ '*', { { '=', TokenType::ASSIGNMENT }, { '\0', TokenType::OPERATOR } } },
		{ '/', { { '=', TokenType::ASSIGNMENT }, { '\0', TokenType::OPERATOR } } },
		{ '%', { { '=', TokenType::ASSIGNMENT }, { '\0', TokenType::OPERATOR } } },

		{ '>', { { '=', TokenType::OPERATOR }, { '\0', TokenType::OPERATOR } } },
		{ '<', { { '=', TokenType::OPERATOR }, { '\0', TokenType::OPERATOR } } },

		{ '|', { { '|', TokenType::OPERATOR } } },
		{ '&', { { '&', TokenType::OPERATOR } } },
		{ '!', { { '=', TokenType::OPERATOR }, { '\0', TokenType::OPERATOR } } },

		{ '.', { { '\0', TokenType::OPERATOR } } },

		{ '=', { { '=', TokenType::OPERATOR }, { '\0', TokenType::ASSIGNMENT } } },

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
	char inString = ' ';
	for (std::size_t a = 0; a < fileLine.length(); ++a)
	{
		// scan strings

		if ((fileLine[a] == '\'' || fileLine[a] == '"') && inString == ' ')
		{
			FindKeyword(file, line, tokens, token);

			inString = fileLine[a];
			continue;
		}
		else if (fileLine[a] != inString && inString != ' ')
		{
			token += fileLine[a];
			continue;
		}
		else if (fileLine[a] == inString && inString != ' ')
		{
			// check string for escape characters
			std::size_t newline = token.find("\\n");
			while (newline != std::string::npos)
			{
				token[newline] = '\n';
				token.erase(newline + 1);

				newline = token.find("\\n", newline + 1);
			}

			tokens.push_back(Token{ file, line, TokenType::STRING_VAL, token });
			token = "";

			inString = ' ';
			continue;
		}

		// ignore comments and whitespace

		if (fileLine[a] == '#')
			break;

		if (fileLine[a] == ' ' || fileLine[a] == '\t')
		{
			FindKeyword(file, line, tokens, token);
			continue;
		}

		// find symbols

		auto symbol = symbols.find(fileLine[a]);
		if (symbol != symbols.end())
		{
			if (symbol->first == '.' && !token.empty() && token.back() - '0' >= 0 && token.back() - '0' <= 9)
			{
				token += fileLine[a];
				continue;
			}

			FindKeyword(file, line, tokens, token);

			for (const auto& match : symbol->second)
			{
				if (match.first != '\0' && a < fileLine.length() - 1 && fileLine[a + 1] == match.first)
				{
					tokens.push_back(Token{ file, line, match.second, std::string(1, fileLine[a]) + match.first });

					a++;
					goto CONTINUE_NEXT;
				}
				else if (match.first == '\0')
				{
					tokens.push_back(Token{ file, line, match.second, std::string(1, fileLine[a]) });

					goto CONTINUE_NEXT;
				}
			}

			throw Error(file, line, std::string("unknown symbol '") + fileLine[a] + "'");
		}

		token += fileLine[a];

		CONTINUE_NEXT:;
	}

	FindKeyword(file, line, tokens, token);

	// allows curly brackets to be optional on single statements
	if (!tokens.empty() &&
		tokens[0].type != TokenType::IF && tokens[0].type != TokenType::ELSE &&
		(tokens[0].type != TokenType::ELSE || tokens[1].type != TokenType::IF) &&
		tokens[0].type != TokenType::WHILE && tokens[0].type != TokenType::FOR &&
		tokens.back().type != TokenType::OPEN_CURLY)
		tokens.push_back(Token{ file, line, TokenType::EOL, "EOF" });

	return tokens;
}