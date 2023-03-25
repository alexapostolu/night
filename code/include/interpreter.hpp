#pragma once

#include "value.hpp"
#include "bytecode.hpp"

#include <unordered_map>
#include <variant>
#include <string>
#include <stack>
#include <tuple>

// debugging with asserts
#define assert_assignment() \
	assert(scope.vars.contains(code->val) \
		&& "variable definitions should be checked in parser"); \
	assert(!s.empty() \
		&& "the stack should not be empty when assigning, should be checked in parser");

// bool flag signals variable
using expr_stack = std::stack<std::tuple<bool, int>>;

struct InterpreterScope
{
	std::unordered_map<int, int> vars;
};

class Interpreter
{
public:
	Interpreter(bytecodes_t const& bytecodes);

public:
	void parse_bytecodes(bytecodes_t const& codes);

private:
	// wrapper for stack.pop()
	int pop(expr_stack& s);

private:
	InterpreterScope scope;
};