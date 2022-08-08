#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Bytecode;
using bytecodes_t = std::unordered_map<std::string, std::vector<Bytecode> const&>;

struct Bytecode
{
	
};