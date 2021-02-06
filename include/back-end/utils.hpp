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

// start iterator at bracket position, advances to close bracket positions
template <typename Unit, typename UnitType, typename Iterator>
void advance_to_close_bracket(const std::vector<Unit>& units,
	const UnitType& open_bracket, const UnitType& close_bracket, const Iterator& index)
{
	int open_bracket_count = 0;
	for (auto it = index; it != units.end(); ++it)
	{
		if (units[index].type == open_bracket)
		{
			open_bracket_count++;
		}
		else if (units[index].type == close_bracket)
		{
			open_bracket_count--;
			if (open_bracket_count == 0)
				return;
		}
	}
}

namespace night {

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

// searches current scope for variable, if not found, moves to upper scope;
// once found, returns its address, or nullptr otherwise
NightVariable* get_variable(
	const std::shared_ptr<NightScope>& scope,
	const std::string& variable_name
);

bool find_type(
	const std::vector<VariableType>& container,
	const VariableType& type
);

bool find_num_types(
	const std::vector<VariableType>& container
);

} // namespace night

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
	const std::vector<Token>& tokens
);

std::string get_var_types_as_str(
	const std::vector<VariableType>& types
);