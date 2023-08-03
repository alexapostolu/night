#pragma once

#include "interpreter_scope.hpp"
#include "bytecode.hpp"

#include <stack>
#include <optional>

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& i, bytecodes_t const& codes);

void push_num(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it);

intpr::Value pop(std::stack<intpr::Value>& s);