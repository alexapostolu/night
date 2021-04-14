#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"

#include <vector>

std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& _tokens)
{
	/*
	std::vector<std::vector<Token> > code(1);
	
	int open_curly_count = 0;
	bool omit_curly_brackets = false;

	for (auto it = tokens.begin(); it != tokens.end(); ++it)
	{
		switch (it->type)
		{
		case TokenType::IF:
			code.back().push_back(*it);

			omit_curly_brackets = true;
			break;
		case TokenType::ELSE:
			code.back().push_back(*it);

			omit_curly_brackets = true;
			break;
		case TokenType::WHILE:
			code.back().push_back(*it);

			omit_curly_brackets = true;
			break;
		case TokenType::FOR:
			code.back().push_back(*it);

			omit_curly_brackets = true;
			break;
		case TokenType::OPEN_CURLY:
			code.back().push_back(*it);

			open_curly_count++;

			if (omit_curly_brackets)
				omit_curly_brackets = false;

			break;
		case TokenType::CLOSE_CURLY:
			code.back().push_back(*it);

			open_curly_count--;
			break;
		case TokenType::EOL:
			if (omit_curly_brackets)
				omit_curly_brackets = false;
			else if (open_curly_count == 0 && std::next(it, 1) != tokens.end())
				code.push_back(std::vector<Token>());
			else if (open_curly_count != 0 && std::next(it, -1)->type != TokenType::OPEN_CURLY) // lexer asserts that tokens.size() > 1
				code.back().push_back(Token{ "", 0, TokenType::EOL, "EOL" });

			break;
		default:
			code.back().push_back(*it);
		}
	}

	return code[0].empty() ? std::vector<std::vector<Token> >() : code;
	*/

	const bool skip_curly_braces = !_tokens.empty() &&
		_tokens[0].type == TokenType::OPEN_CURLY &&
		_tokens.back().type == TokenType::CLOSE_CURLY;

	const std::vector<Token> tokens(
		_tokens.begin() + skip_curly_braces,
		_tokens.end() - skip_curly_braces);

	std::vector<std::vector<Token> > code;

	int open_curly_count = 0;
	for (auto it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (it == tokens.begin())
			code.push_back(std::vector<Token>());

		if (it->type == TokenType::EOL && open_curly_count == 0)
		{
			if (std::next(it, 1) != tokens.end())
				code.push_back(std::vector<Token>());

			continue;
		}

		if (it->type == TokenType::OPEN_CURLY)
			open_curly_count++;
		else if (it->type == TokenType::CLOSE_CURLY)
			open_curly_count--;

		code.back().push_back(*it);
	}

	return code;
}

bool find_num_types(const VariableTypeContainer& container)
{
	return container.contains(VariableType::INT) &&
		container.contains(VariableType::FLOAT);
}

std::string get_var_types_as_str(const VariableTypeContainer& var_types_set)
{
	assert(!var_types_set.empty());

	std::vector<VariableType> var_types(
		var_types_set.begin(), var_types_set.end());

	std::string str_types = "";

	for (int a = 0; a < (int)var_types.size() - 1; ++a)
		str_types += "'" + var_types[a].to_str() + "', ";

	str_types = var_types.size() > 1
		? "types: " + str_types + "and "
		: "type: " + str_types;

	str_types += "'" + var_types.back().to_str() + "'";

	return str_types;
}