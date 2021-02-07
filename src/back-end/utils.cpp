#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"

#include <vector>

std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& tokens)
{
	std::vector<std::vector<Token> > code;
	for (std::size_t a = 0, openCurlyCount = 0; a < tokens.size(); ++a)
	{
		if (a == 0)
			code.push_back(std::vector<Token>());

		if (tokens[a].type == TokenType::EOL && openCurlyCount == 0)
		{
			if (a < tokens.size() - 1)
				code.push_back(std::vector<Token>());

			continue;
		}

		if (tokens[a].type == TokenType::OPEN_CURLY)
			openCurlyCount++;
		else if (tokens[a].type == TokenType::CLOSE_CURLY)
			openCurlyCount--;

		code.back().push_back(tokens[a]);
	}

	return code;
}

bool find_type(const std::vector<VariableType>& container, const VariableType& type)
{
	for (const VariableType& var_type : container)
	{
		if (var_type == type)
			return true;
	}

	return false;
}

bool find_num_types(const std::vector<VariableType>& container)
{
	return find_type(container, VariableType::INT) ||
		   find_type(container, VariableType::FLOAT);
}

std::string get_var_types_as_str(const std::vector<VariableType>& var_types)
{
	std::string str_types = "";
	for (int a = 0; a < var_types.size() - 1; ++a)
		str_types += "'" + var_types[a].to_str() + "', ";

	if (var_types.size() > 1)
		str_types += "and ";

	str_types += "'" + var_types.back().to_str() + "'";

	return str_types;
}