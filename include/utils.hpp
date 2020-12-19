#pragma once

#include "token.hpp"

#include <memory>
#include <algorithm>
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
   
// converts a variable type to a string
std::string ttos(const VariableType& type);

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

// finds object in array using its name attribute
// returns object's address if found, otherwise returns NULL
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

template <typename T>
bool find_container(const std::vector<T>& container, const std::string& name)
{
	return std::find_if(container.begin(), container.end(), [&](const T& object) {
		return name == object.name;
	}) != container.end();
}

// finds element in array
template <typename T>
bool find_type(const std::vector<T>& container, const T& data)
{
	return std::find_if(container.begin(), container.end(), [&](const T& object) {
		return data == object;
	}) != container.end();
}

} // namespace night

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
	const std::vector<Token>& tokens
);

// extracts expression from tokens; returns a type checked expression
std::shared_ptr<Expression> ParseTokenExpression(
	const std::vector<Token>& tokens,

	const std::size_t start,
	const std::size_t end,

	const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions,
	const std::vector<CheckClass>&    classes,

	const std::vector<CheckVariable>& parameters,

	std::vector<VariableType>* types = nullptr
);

// extracts condition from tokens; returns a type checked expression
std::shared_ptr<Expression> ExtractCondition(
	const std::vector<Token>& tokens,
	std::size_t& closeBracketIndex,

	const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions,
	const std::vector<CheckClass>&    classes,

	const std::vector<CheckVariable>& parameters,

	const std::string& stmt
);

// extracts body from tokens; returns a parsed statement vector
std::vector<Statement> ExtractBody(
	const std::vector<Token>& tokens,
	const std::size_t closeBracketIndex,

	std::vector<CheckVariable>& variables, // can't be const since variables need to be removed after scope finished

	const std::string& stmt
);