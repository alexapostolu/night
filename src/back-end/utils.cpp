#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"

#include <vector>

std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& tokens)
{
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
	return container.find(VariableType::INT) != container.end() ||
		   container.find(VariableType::FLOAT) != container.end();
}

std::string get_var_types_as_str(const VariableTypeContainer& var_types_set)
{
	assert(!var_types_set.empty());

	std::vector<VariableType> var_types(var_types_set.begin(), var_types_set.end());

	std::string str_types = "";
	for (int a = 0; a < (int)var_types.size() - 1; ++a)
		str_types += "'" + var_types[a].to_str() + "', ";

	str_types = var_types.size() > 1
		? "types: " + str_types + "and "
		: "type: " + str_types;

	str_types += "'" + var_types.back().to_str() + "'";

	return str_types;
}