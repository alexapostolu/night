#pragma once

#include "token.hpp"

#include <variant>
#include <string>
#include <vector>

struct NightData
{
	VariableType type;

	std::variant<
		bool, int, float,
		std::string
	> data;

	std::vector<NightData> extras;

	bool is_num() const;
	std::string to_str() const;

	// if type is an integer or float,
	// return data
	//
	// use only if type could be an int or float
	//
	// if type is CERTAINLY an int, then use std::get<int>()
	float get_num() const;
};

struct NightVariable
{
	std::string name;
	NightData value;
};

struct NightFunction
{
	std::string name;
	std::vector<std::string> params;
	std::vector<Statement> body;
};

struct NightClass
{
	std::string name;
	std::vector<NightVariable> variables;
	std::vector<NightVariable> methods;
};

struct NightScope
{
	NightScope* upper_scope;
	std::vector<NightVariable> night_variables;

	NightScope() = delete;
};

// displays standard output
void NightPrint(const NightData& value);

bool compare_array(const NightData& arr1, const NightData& arr2)
{
	if (arr1.extras.size() != arr2.extras.size())
		return false;

	for (std::size_t a = 0; a < arr1.extras.size(); ++a)
	{
		if (arr1.extras[a].type != arr2.extras[a].type)
			return false;
		if (arr1.extras[a].type == VariableType::ARRAY && !compare_array(arr1.extras[a], arr2.extras[a]))
			return false;

	}

	return true;
}