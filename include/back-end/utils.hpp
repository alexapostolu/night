#pragma once

#include "token.hpp"
#include "night.hpp"
#include "../error.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// index starts right after opening bracket
// advances index to closing bracket
template <typename Unit, typename UnitType>
void AdvanceToCloseBracket(const std::vector<Unit>& units,
	const UnitType& openBracket, const UnitType& closeBracket, std::size_t& index)
{
	for (int openBracketCount = 0; index < units.size(); ++index)
	{
		if (units[index].type == openBracket)
		{
			openBracketCount++;
		}
		else if (units[index].type == closeBracket)
		{
			if (openBracketCount == 0)
				return;

			openBracketCount--;
		}
	}
}

namespace night {

// finds object in array using its 'name' attribute,
// if found, returns 'true', otherwise returns 'false'
template <typename T>
bool find_container(const std::vector<T>& container, const std::string& name)
{
	for (const T& object : container)
	{
		if (name == object.name)
			return true;
	}

	return false;
}

// finds object in array using its 'name' attribute,
// if found, returns its address, otherwise returns 'nullptr'
template <typename T>
T* get_container(std::vector<T>& container, const std::string& name)
{
	for (T& object : container)
	{
		if (name == object.name)
			return &object;
	}

	return nullptr;
}

bool find_variable(
	const Scope& scope,
	const std::string& name
);

CheckVariable* get_variable(
	Scope& scope,
	const std::string& var_name
);

const CheckVariable* get_variable(
	const Scope& scope,
	const std::string& var_name
);

bool find_type(
	const std::vector<VariableType>& container,
	const VariableType& type
);

bool find_num_types(
	const std::vector<VariableType>& container
);

NightVariable* get_variable(
	NightScope& scope,
	const std::string& var_name
);

} // namespace night

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
	const std::vector<Token>& tokens
);