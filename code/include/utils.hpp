#pragma once

#include "value_type.hpp"

#include <source_location>
#include <string>

ValueType token_var_type_to_val_type(std::string const& type, std::source_location const& s_loc = std::source_location::current());