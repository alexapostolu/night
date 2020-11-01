#pragma once

#include "token.h"

#include <regex>
#include <iterator>
#include <string>
#include <vector>
#include <unordered_map>

void FindKeyword(std::unordered_map<std::string, TokenType>& keywords, std::vector<Token>& tokens, std::string& token,
	const std::string& file, int line)
{
	auto findKeyword = keywords.find(token);
	if (findKeyword != keywords.end())
		tokens.push_back(Token{ file, line, findKeyword->second, token });
	else if (std::regex_match(token, std::regex("[0-9]+")))
		tokens.push_back(Token{ file, line, TokenType::NUM_VAL, token });
	else if (std::regex_match(token, std::regex("((\\+|-)?([0-9]+)(\\.[0-9]+)?)|((\\+|-)?\\.?[0-9]+)")))
		tokens.push_back(Token{ file, line, TokenType::NUM_VAL, token });
	else if (std::regex_match(token, std::regex("[a-zA-Z_][a-zA-Z_0-9]*")))
		tokens.push_back(Token{ file, line, TokenType::VARIABLE, token });
	else
		return;

	token = "";
}

std::vector<Token> Lexer(const std::string& file, int line, const std::string& fileLine)
{
	std::unordered_map<std::string, TokenType> symbols{
		{ "||", TokenType::OPERATOR },
		{ "&&", TokenType::OPERATOR },
		{ "(", TokenType::OPEN_BRACKET },
		{ ")", TokenType::CLOSE_BRACKET },
		{ "[", TokenType::OPEN_SQUARE },
		{ "]", TokenType::CLOSE_SQUARE },
		{ "{", TokenType::OPEN_CURLY },
		{ "}", TokenType::CLOSE_CURLY },
		{ ",", TokenType::COMMA }
	};

	std::unordered_map<char, std::pair<TokenType, TokenType> > doubleSymbols{
		{ '>', { TokenType::OPERATOR, TokenType::OPERATOR } },
		{ '<', { TokenType::OPERATOR, TokenType::OPERATOR } },
		{ '!', { TokenType::OPERATOR, TokenType::OPERATOR } },
		{ '+', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '-', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '*', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '/', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '%', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '=', { TokenType::ASSIGNMENT, TokenType::ASSIGNMENT } }
	};

	std::unordered_map<std::string, TokenType> keywords{
		{ "true", TokenType::BOOL_VAL },
		{ "false", TokenType::BOOL_VAL },
		{ "set", TokenType::SET },
		{ "if", TokenType::IF },
		{ "else", TokenType::ELSE },
		{ "while", TokenType::WHILE },
		{ "for", TokenType::FOR },
		{ "import", TokenType::IMPORT },
		{ "include", TokenType::IMPORT },
	};

	std::vector<Token> tokens;
	std::string token = "";
	for (std::size_t a = 0; a < fileLine.length(); ++a)
	{
		auto findSymbol = symbols.find(std::string(1, fileLine[a]));
		if (findSymbol != symbols.end())
		{
			FindKeyword(keywords, tokens, token, file, line);

			tokens.push_back(Token{ file, line, findSymbol->second, token });

			continue;
		}

		auto findDoubleSymbol = std::find_if(doubleSymbols.begin(), doubleSymbols.end(),
			[&](const std::pair<char, std::pair<TokenType, TokenType> >& symbol) { return symbol.first == fileLine[a]; });
		if (findDoubleSymbol != doubleSymbols.end())
		{
			FindKeyword(keywords, tokens, token, file, line);

			if (a < fileLine.length() - 1 && fileLine[a + 1] == '=')
			{
				tokens.push_back(Token{
					file, line,
					findDoubleSymbol->second.second,
					findDoubleSymbol->first + "="
				});
			}
			else
			{
				tokens.push_back(Token{
					file, line,
					findDoubleSymbol->second.first,
					std::string(1, findDoubleSymbol->first)
				});
			}

			continue;
		}

		token += fileLine[a];
	}

	tokens.push_back(Token{ "", 0, TokenType::EOL });
	return tokens;
}