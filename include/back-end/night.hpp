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
};

// displays standard output
void NightPrint(const NightData& value);

bool compare_array(const NightData& arr1, const NightData& arr2);