#pragma once

#include "bytecode.hpp"
#include "ast/ast.hpp"

bytecodes_t code_gen(AST_Block const& block);