#pragma once

#include "value.hpp"
#include "bytecode.hpp"

#include <unordered_map>
#include <functional>
#include <variant>
#include <string>
#include <stack>
#include <tuple>

// bool flag signals variable
using expr_stack = std::stack<std::tuple<bool, int>>;
using var_container = std::unordered_map<int, Value>;

struct Function;
using func_container = std::unordered_map<std::string, Function>;

struct Function
{
	std::vector<int> params;
	bytecodes_t codes;
};

struct Interpreter
{
	var_container global_vars;
	func_container funcs;
	expr_stack s;
};

void interpret_bytecodes(Interpreter const& interpreter, bytecodes_t const& codes);

// wrapper for stack.pop()
int pop(InterpreterScope& scope, expr_stack& s);