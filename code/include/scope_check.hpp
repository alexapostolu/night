#pragma once

#include "parser_scope.hpp"

#include <string>

bool check_variable_defined(ParserScope const& scope, std::string const& var_name, Location const& loc);