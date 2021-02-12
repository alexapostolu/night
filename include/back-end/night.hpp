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

struct NightFunction
{
	std::string name;
	std::vector<std::string> params;
	std::vector<Statement> body;

	bool operator==(const std::string& _name) const;
};

struct NightScope
{
	const std::shared_ptr<NightScope> upper_scope;
	std::unordered_map<std::string, NightData> night_variables;

	NightScope(const std::shared_ptr<NightScope>& _upper_scope);
};

// displays standard output
void NightPrint(const NightData& value);

bool compare_array(const NightData& arr1, const NightData& arr2);