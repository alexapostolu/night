#pragma once

#include "token.h"
#include "error.h"

#include <regex>
#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>

void FindKeyword(const std::string& file, int line, const std::unordered_map<std::string, TokenType>& keywords,
	std::vector<Token>& tokens, std::string& token)
{
	if (token.length() == 0)
		return;

	auto findKeyword = keywords.find(token);
	if (findKeyword != keywords.end())
		tokens.push_back(Token{ file, line, findKeyword->second, token });
	else if (std::regex_match(token, std::regex("((\\+|-)?([0-9]+)(\\.[0-9]+)?)|((\\+|-)?\\.?[0-9]+)")))
		tokens.push_back(Token{ file, line, TokenType::NUM_VAL, token });
	else if (std::regex_match(token, std::regex("[a-zA-Z_][a-zA-Z_0-9]*")))
		tokens.push_back(Token{ file, line, TokenType::VARIABLE, token });
	else
		throw Error(file, line, "unidentified token '" + token + "'");

	token = "";
}

std::vector<Token> Lexer(const std::string& file, int line, const std::string& fileLine)
{
	const std::unordered_map<std::string, TokenType> symbols{
		{ "||", TokenType::OPERATOR },
		{ "&&", TokenType::OPERATOR },
		{ "(", TokenType::OPEN_BRACKET },
		{ ")", TokenType::CLOSE_BRACKET },
		{ "[", TokenType::OPEN_SQUARE },
		{ "]", TokenType::CLOSE_SQUARE },
		{ "{", TokenType::OPEN_CURLY },
		{ "}", TokenType::CLOSE_CURLY },
		{ ":", TokenType::COLON },
		{ ",", TokenType::COMMA }
	};

	const std::unordered_map<char, std::pair<TokenType, TokenType> > doubleSymbols{
		{ '>', { TokenType::OPERATOR, TokenType::OPERATOR } },
		{ '<', { TokenType::OPERATOR, TokenType::OPERATOR } },
		{ '!', { TokenType::OPERATOR, TokenType::OPERATOR } },
		{ '+', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '-', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '*', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '/', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '%', { TokenType::OPERATOR, TokenType::ASSIGNMENT } },
		{ '=', { TokenType::ASSIGNMENT, TokenType::OPERATOR } }
	};

	const std::unordered_map<std::string, TokenType> keywords{
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
	char inString = ' ';
	for (std::size_t a = 0; a < fileLine.length(); ++a)
	{
		// scan strings

		if ((fileLine[a] == '\'' || fileLine[a] == '"') && inString == ' ')
		{
			FindKeyword(file, line, keywords, tokens, token);

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
			size_t newline = token.find("\\n");
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
			FindKeyword(file, line, keywords, tokens, token);
			continue;
		}

		// scan special symbols

		if (a < fileLine.length() - 1 && fileLine[a] == '<' && fileLine[a + 1] == '-')
		{
			FindKeyword(file, line, keywords, tokens, token);
			tokens.push_back(Token{ file, line, TokenType::ARROW, "<-" });

			a++;
			continue;
		}
		if (a < fileLine.length() - 1 && fileLine[a] == '.' && fileLine[a + 1] == '.')
		{
			FindKeyword(file, line, keywords, tokens, token);
			tokens.push_back(Token{ file, line, TokenType::RANGE, ".." });

			a++;
			continue;
		}

		// scan symbols

		auto findSymbol = std::find_if(symbols.begin(), symbols.end(), [&](const std::pair<std::string, TokenType>& symbol) {
			if (fileLine[a] == symbol.first[0] && symbol.first.length() == 2 && (a == fileLine.length() - 1 || fileLine[a + 1] != symbol.first[1]))
				throw Error(file, line, "unexpected symbol '" + std::string(1, fileLine[a]) + "'");
			return symbol.first[0] == fileLine[a];
		});
		if (findSymbol != symbols.end())
		{
			FindKeyword(file, line, keywords, tokens, token);
			tokens.push_back(Token{ file, line, findSymbol->second, findSymbol->first });

			if (findSymbol->first.length() == 2)
				a++;

			continue;
		}

		// scan double symbols

		auto findDoubleSymbol = std::find_if(doubleSymbols.begin(), doubleSymbols.end(),
			[&](const std::pair<char, std::pair<TokenType, TokenType> >& symbol) { return symbol.first == fileLine[a]; });
		if (findDoubleSymbol != doubleSymbols.end())
		{
			FindKeyword(file, line, keywords, tokens, token);
			
			/*
			if (findDoubleSymbol->first == '-' && a < fileLine.length() - 1 && fileLine[a + 1] - '0' >= 0 && fileLine[a + 1] - '0' <= 9)
			{
				token += fileLine[a];
				continue;
			}
			*/

			if (a < fileLine.length() - 1 && fileLine[a + 1] == '=')
			{
				tokens.push_back(Token{
					file, line,
					findDoubleSymbol->second.second,
					std::string(1, findDoubleSymbol->first) + "="
				});

				a++;
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

	FindKeyword(file, line, keywords, tokens, token);
	
	if (tokens.empty())
		return tokens;

	if (tokens[0].type != TokenType::IF && tokens[0].type != TokenType::ELSE &&
		(tokens[0].type != TokenType::ELSE || tokens[1].type != TokenType::IF) &&
		tokens[0].type != TokenType::WHILE && tokens[0].type != TokenType::FOR &&
		tokens.back().type != TokenType::OPEN_CURLY)
		tokens.push_back(Token{ file, line, TokenType::EOL, "EOF" });

	return tokens;
}