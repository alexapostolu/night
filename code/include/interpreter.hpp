#pragma once

#include "value.hpp"
#include "bytecode.hpp"

#include <unordered_map>
#include <stack>
#include <tuple>
#include <variant>
#include <string>

using expr_stack = std::stack<int>;

// <id, val>
using var_container = std::unordered_map<bytecode_t, int>;

struct InterpreterFunction;
using func_container = std::unordered_map<std::string, InterpreterFunction>;

struct InterpreterFunction
{
	std::vector<int> params;
	bytecodes_t codes;
};

struct Interpreter
{
public:
	Interpreter(bytecodes_t& _codes);

	void interpret_bytecodes();

private:
	void push_num(bytecodes_t::const_iterator& it);
	void push_char(bytecodes_t::const_iterator& it);
	void push_var(bytecodes_t::const_iterator& it);
	int pop(Interpreter& scope, expr_stack& s);

public:
	func_container funcs;
	var_container vars;
	expr_stack s;

private:
	bytecodes_t& codes;
};
