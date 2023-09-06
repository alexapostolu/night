#pragma once

#include "parser_scope.hpp"

#include <string>

bool check_variable_defined(ParserScope const& scope, std::string const& name, Location const& loc);
bool check_function_defined(ParserScope const& scope, std::string const& name, Location const& loc);