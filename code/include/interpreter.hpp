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

struct InterpreterScope
{
	std::unordered_map<int, Value> vars;
};

class Interpreter
{
public:
	Interpreter(bytecodes_t const& bytecodes);

public:
	void interpret_bytecodes(bytecodes_t const& codes);

private:
	// wrapper for stack.pop()
	int pop(expr_stack& s);

private:
	InterpreterScope scope;
	expr_stack s;
};