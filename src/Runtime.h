#pragma once

#include <vector>

#include "DataTypes/Error.h"
#include "DataTypes/Token.h"
#include "DataTypes/Variable.h"

void Runtime(const std::vector<std::vector<Token> >& code, const std::vector<Variable>& variables)
{
	for (std::size_t a = 0; a < code.size(); ++a)
	{
		if (code[a].size() >= 6 && code[a][0].type == TokenType::IF)
		{
			if (code[a][2].token == "true")
			{
				std::vector<std::vector<Token> > temp;

				int curlyBracket = 0, start = 5;
				for (std::size_t b = start; b < code[a].size(); ++b)
				{
					if (code[a][b].type == TokenType::OPEN_CURLY)
						curlyBracket += 1;
					else if (code[a][b].type == TokenType::CLOSE_CURLY)
						curlyBracket -= 1;

					if ((code[a][b].type == TokenType::SEMICOLON ||
						code[a][b].type == TokenType::CLOSE_CURLY) && curlyBracket == 0)
					{
						std::vector<Token> temp1(code[a].begin() + start, code[a].begin() + b + 1);
						
						start = b + 1;

						temp.push_back(temp1);
					}
				}

				Runtime(temp, variables);
			}
		}
		else if (code[a].size() >= 7 && code[a][0].type == TokenType::ELSE &&
			code[a][1].type == TokenType::IF)
		{
			int conditionIndex;

			try {
				if (code.at(a - 1)[0].type != TokenType::IF || code[a - 1][0].type != TokenType::ELSE)
					throw "";

				if (code[a - 1][0].type == TokenType::IF)
					conditionIndex = 2;
				else
					conditionIndex = 3;
			}
			catch (...) {
				throw Error(night::_invalid_if_statement_, code[a], 0, 4,
					"statement 'else if' must come after statement 'if'");
			}

			if (code[a - 1][conditionIndex].token == "false" && code[a][3].token == "true")
			{
				std::vector<std::vector<Token> > temp;

				int curlyBracket = 0, start = 5;
				for (std::size_t b = start; b < code[a].size(); ++b)
				{
					if (code[a][b].type == TokenType::OPEN_CURLY)
						curlyBracket += 1;
					else if (code[a][b].type == TokenType::CLOSE_CURLY)
						curlyBracket -= 1;

					if ((code[a][b].type == TokenType::SEMICOLON ||
						code[a][b].type == TokenType::CLOSE_CURLY) && curlyBracket == 0)
					{
						std::vector<Token> temp1(code[a].begin() + start, code[a].begin() + b + 1);

						start = b + 1;

						temp.push_back(temp1);
					}
				}

				Runtime(temp, variables);
			}
		}
		else if (code[a].size() >= 3 && code[a][0].type == TokenType::ELSE)
		{
			int conditionIndex;

			try {
				if (code.at(a - 1)[0].type != TokenType::IF && code[a - 1][0].type != TokenType::ELSE)
					throw "";

				if (code[a - 1][0].type == TokenType::IF)
					conditionIndex = 2;
				else
					conditionIndex = 3;
			}
			catch (...) {
				throw Error(night::_invalid_if_statement_, code[a], 0, 3,
					"statement 'else' must come after statement 'if'");
			}

			if (code[a - 1][conditionIndex].token == "false")
			{
				std::vector<std::vector<Token> > temp;

				int curlyBracket = 0, start = 2;
				for (std::size_t b = start; b < code[a].size(); ++b)
				{
					if (code[a][b].type == TokenType::OPEN_CURLY)
						curlyBracket += 1;
					else if (code[a][b].type == TokenType::CLOSE_CURLY)
						curlyBracket -= 1;

					if ((code[a][b].type == TokenType::SEMICOLON ||
						code[a][b].type == TokenType::CLOSE_CURLY) && curlyBracket == 0)
					{
						std::vector<Token> temp1(code[a].begin() + start, code[a].begin() + b + 1);

						start = b + 1;

						temp.push_back(temp1);
					}
				}

				Runtime(temp, variables);
			}
		}
	}
}

void Setup(const std::vector<Token>& tokens, const std::vector<Variable>& variables, bool runtime)
{
	static std::vector<std::vector<Token> > code;

	if (runtime)
		Runtime(code, variables);
	else
		code.push_back(tokens);
}