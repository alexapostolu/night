#pragma once

#include "token.hpp"
#include "../error.hpp"
#include "night.hpp"

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

// finds object in array using its name attribute
// returns object's address if found, otherwise returns NULL
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

// finds element in array
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

bool find_variable(
	const Scope& scope,
	const std::string& name
);

// since the implementation is the same, make this into a template?
const CheckVariable* get_variable(
	const Scope& scope,
	const std::string& var_name
);

// do I need dis?
//
//
//
//
//
//
CheckVariable* get_variable(
	Scope& scope,
	const std::string& var_name
);

template <typename Type, typename... Types>
bool find_type(const std::vector<VariableType>& container, Type type, Types... types)
{
	for (const VariableType& var_type : container)
	{
		if (var_type == type && type == VariableType::CLASS && var_type.class_name == type.class_name)
			return true;
	}

	return find_type(container, types);
}

bool find_type(const std::vector<VariableType>& container) { return false; }

NightVariable* get_variable(NightScope& scope, const std::string& var_name);

} // namespace night

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
	const std::vector<Token>& tokens
);