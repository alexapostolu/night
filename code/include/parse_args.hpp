#pragma once

#include <vector>
#include <string>

// Applys any flags such as debug -d,
// and returns the starting file name
std::string parse_args(std::vector<std::string_view> const& args);