#pragma once

#include "type.hpp"

#include <string>
#include <unordered_map>

using type_container = std::unordered_map<std::string, ValueType>;

struct Scope
{
	type_container vars;
};