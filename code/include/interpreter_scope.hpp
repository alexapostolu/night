#pragma once

#include "bytecode.hpp"

#include <unordered_map>
#include <vector>

namespace intpr
{

struct Value;

struct Array
{
	Value* data;
	std::size_t size;
};

struct Value
{
	union {
		int64_t i;
		uint64_t ui;
		double d;
		char* s;
		Array a;
	} as;

	Value() = default;
	Value(int64_t _i);
	Value(uint64_t _ui);
	Value(double _d);
	Value(char* _s);
	Value(Array _a);
	Value(Value const& _v);
	~Value();
};

} // intpr""

namespace night {
	using id_t = uint64_t;
}

// <id, val>
using var_container = std::unordered_map<id_t, intpr::Value>;

struct InterpreterFunction;
using func_container = std::unordered_map<uint64_t, InterpreterFunction>;

struct InterpreterFunction
{
	std::vector<uint64_t> param_ids;
	bytecodes_t codes;
};

class InterpreterScope
{
public:
	static func_container funcs;

	InterpreterScope() = default;
	InterpreterScope(InterpreterScope const& parent);

	intpr::Value& get_variable(uint64_t id);

	void set_variable(uint64_t id, intpr::Value const& val, bool is_global);

private:
	static var_container global_variables;

	var_container vars;
};
