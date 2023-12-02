#include "interpreter_scope.hpp"

func_container InterpreterScope::funcs = {};

intpr::Value::Value(int64_t _i) { as.i = _i; }
intpr::Value::Value(uint64_t _ui) { as.ui = _ui; }
intpr::Value::Value(double _d) { as.d = _d; }
intpr::Value::Value(char* _s) { as.s = _s; }
intpr::Value::Value(Array _a)
{
	delete[] as.a.data;

	as.a.data = new intpr::Value[_a.size];
	for (int i = 0; i < _a.size; ++i)
		as.a.data[i] = _a.data[i];
}

intpr::Value::Value(Value const& _v)
{
	as.i = _v.as.i;
	as.ui = _v.as.ui;
	as.d = _v.as.d;
	as.s = _v.as.s;
	as.a = _v.as.a;
}

int InterpreterScope::new_id() { static int id = 7; return ++id; }