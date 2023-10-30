#pragma once

#include "ast/statement.hpp"
#include "bytecode.hpp"

#include <vector>

bytecodes_t code_gen(std::vector<stmt_p>& block);