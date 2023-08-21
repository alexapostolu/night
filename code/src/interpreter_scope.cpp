#include "interpreter_scope.hpp"
#include <string>

func_container InterpreterScope::funcs = {};

intpr::Value::Value(int64_t _i)
	: type(ValueType::INT), i(_i) {}

intpr::Value::Value(uint64_t _ui)
	: type(ValueType::UINT), ui(_ui) {}

intpr::Value::Value(float _f)
	: type(ValueType::FLOAT), f(_f) {}

intpr::Value::Value(std::string _s)
	: type(ValueType::STR), s(_s) {}

intpr::Value::Value(std::vector<Value> const& _v)
	: type(ValueType::ARR), v(_v) {}

intpr::Value::Value(Value const& _v)
	: type(_v.type), i(_v.i), f(_v.f), s(_v.s), v(_v.v) {}

int InterpreterScope::new_id() { static int id = 7; return ++id; }