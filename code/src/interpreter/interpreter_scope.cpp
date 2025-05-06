#include "interpreter/interpreter_scope.hpp"

#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <assert.h>

func_container InterpreterScope::funcs = {};

intpr::Value::Value(int64_t _i) { as.i = _i; }

intpr::Value::Value(uint64_t _ui) { as.ui = _ui; }

intpr::Value::Value(double _d) { as.d = _d; }

intpr::Value::Value(char* _s)
{
	assert(_s);

	size_t len = strlen(_s);

	as.s = (char*)malloc((len + 1) * sizeof(char));
	if (!as.s)
		exit(1);

	strncpy(as.s, _s, len);
	as.s[len] = '\0';
}

intpr::Value::Value(Array _a)
{
	as.a = _a;
}

intpr::Value::Value(Value const& _v)
{
	as = _v.as;
}

intpr::Value::~Value()
{
	
};

InterpreterScope::InterpreterScope(InterpreterScope const& parent)
{
	vars = parent.vars;
}

intpr::Value& InterpreterScope::get_variable(night::id_t id)
{
	if (vars.contains(id))
		return vars[id];

	assert(parent);
	return parent->get_variable(id);
}

void InterpreterScope::set_variable(night::id_t id, intpr::Value const& val)
{
	if (vars.contains(id))
		vars[id] = val;

	assert(parent);
	return parent->set_variable(id, val);
}
