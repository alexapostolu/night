#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/back-end/night.hpp"

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

bool night::find_variable(const Scope& scope, const std::string& name)
{
	const Scope* current_scope = &scope;
	while (current_scope != nullptr)
	{
		for (const CheckVariable& variable : current_scope->check_variables)
		{
			if (name == variable.name)
				return true;
		}

		current_scope = current_scope->upper_scope;
	}

	return false;
}

NightVariable* night::get_variable(NightScope& scope, const std::string& var_name)
{
	NightScope* current_scope = &scope;
	while (current_scope != nullptr)
	{
		for (NightVariable& night_var : current_scope->night_variables)
		{
			if (var_name == night_var.name)
				return &night_var;
		}

		current_scope = current_scope->upper_scope;
	}

	return nullptr;
}

const CheckVariable* night::get_variable(const Scope& scope, const std::string& var_name)
{
	const Scope* current_scope = &scope;
	while (current_scope != nullptr)
	{
		for (const CheckVariable& check_var : current_scope->check_variables)
		{
			if (var_name == check_var.name)
				return &check_var;
		}

		current_scope = current_scope->upper_scope;
	}

	return nullptr;
}

CheckVariable* night::get_variable(Scope& scope, const std::string& var_name)
{
	Scope* current_scope = &scope;
	while (current_scope != nullptr)
	{
		for (CheckVariable& check_var : current_scope->check_variables)
		{
			if (var_name == check_var.name)
				return &check_var;
		}

		current_scope = current_scope->upper_scope;
	}

	return nullptr;
}

bool night::find_type(const std::vector<VariableType>& container, const VariableType& type)
{
	for (const VariableType& var_type : container)
	{
		if (var_type == type)
			return true;
	}

	return false;
}

bool night::find_num_types(const std::vector<VariableType>& container)
{
	return find_type(container, VariableType::INT) ||
		   find_type(container, VariableType::FLOAT);
}