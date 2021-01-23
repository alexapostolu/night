#pragma once

#include "token.hpp"
#include "error.hpp"

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

template <typename T>
const T* get_container(const std::vector<T>& container, const std::string& name)
{
	for (const T& object : container)
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

// finds enum type in array
template <typename T>
bool find_type(const std::vector<VariableType>& container, const T& var_type)
{
	for (const VariableType& contain_type : container)
	{
		if ((var_type.type == VariableType::CLASS && contain_type.type == VariableType::CLASS && var_type.name == contain_type.name) ||
			(var_type.type == contain_type.type))
			return true;
	}

	return false;
}

template <typename T>
std::vector<T> access(const std::vector<T>& container, std::size_t start, std::size_t end)
{
	return std::vector<T>(
		std::begin(container) + start,
		std::begin(container) + end
	);
}

} // namespace night

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
	const std::vector<Token>& tokens
);