#pragma once

#include "statement_scope.hpp"

#include <string>

bool check_variable_defined(StatementScope& scope, std::string const& name, Location const& loc);
bool check_function_defined(StatementScope const& scope, std::string const& name, Location const& loc);