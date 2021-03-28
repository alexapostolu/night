#pragma once

#include "token.hpp"

#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

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

using NightVariableContainer = std::unordered_map<std::string, NightData>;

struct NightFunction
{
	std::vector<std::string> params;
	std::vector<Statement> body;
};

using NightFunctionContainer = std::unordered_map<std::string, NightFunction>;



// displays standard output
void NightPrint(const NightData& value);

bool compare_array(const NightData& arr1, const NightData& arr2);