#pragma once

#include <string>
#include <unordered_map>

using type_container = std::unordered_map<std::string, std::string>;

struct Scope
{
	type_container vars;
};