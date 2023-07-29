#pragma once

#include "interpreter_scope.hpp"
#include "bytecode.hpp"

#include <functional>
#include <unordered_map>
#include <stack>
#include <tuple>
#include <variant>
#include <string>

using expr_stack = std::stack<int>;

struct Interpreter
{
public:
	void push_num(bytecodes_t::const_iterator& it);
	void push_bool(bytecodes_t::const_iterator& it);
	void push_char(bytecodes_t::const_iterator& it);
	void push_var(bytecodes_t::const_iterator& it);
	int pop();

public:
	static func_container funcs;
	var_container vars;
	expr_stack s;
};

void interpret_bytecodes(bytecodes_t const& codes);