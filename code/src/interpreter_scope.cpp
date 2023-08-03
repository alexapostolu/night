#include "interpreter_scope.hpp"
#include <string>

func_container InterpreterScope::funcs = {};

intpr::Value::Value(intpr::ValueType _type, int _i)
	: type(_type), i(_i) {}

intpr::Value::Value(float _f)
	: type(ValueType::FLOAT), f(_f) {}

intpr::Value::Value(std::string _s)
	: type(ValueType::STR), s(_s) {}

intpr::Value::Value(Value const& _v)
	: type(_v.type), i(_v.i), f(_v.f), s(_v.s) {}