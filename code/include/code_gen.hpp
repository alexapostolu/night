#pragma once

#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "ast/ast.hpp"

bytecodes_t code_gen(AST_Block const& block, ParserScope& scope);