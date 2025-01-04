#pragma once

#include "ast/statement.hpp"

#include "bytecode.h"

#include <vector>

bytes_t code_gen(std::vector<stmt_p>& block);