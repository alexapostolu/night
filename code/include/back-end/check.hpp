#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Type
{
	enum T {
		BOOL,
		INT, FLOAT,
		STR, ARR,
		//TYPE
	} type;

	Type(T _type);
	Type(T _type, std::vector<Type> const& _elem_types);

	bool operator==(Type _t) const;

	std::string to_str() const;

	std::vector<Type> elem_types;
};

using TypeContainer = std::vector<Type>;

struct CheckVariable
{
	// a note about parameters:
	/*
	// to perform type checking, parameters' types must be evaluated when the
	// function is defined
	//
	// they are stored in the same container as normal variables, so the only
	// difference is that they don't have a type
	//
	// they can be differentiated from normal variables using the method:
	// 'needs_types()'
	//
	// their types are giving to them through the expressions they encounter,
	// for example 'param || true' would mean 'param' is a boolean
	//
	// if a parameter still doesn't have a type at the end of the function,
	// then it is given all the types
	//
	// once a parameter has types, it then behaves like a normal variable
	*/
	TypeContainer types;
};

using CheckVariableContainer = std::unordered_map<std::string, CheckVariable>;

struct CheckFunction
{
	std::vector<TypeContainer> param_types;

	// a note about return types:
	/*
	// function return types have to be deduced when they are defined
	//
	// this is done by examining the return statement(s) of the function
	//
	// if no return types can be deduced, then the return type is treated as
	// whatever type is required for it to be
	*/
	TypeContainer rtn_types;

	bool is_void;
};

using CheckFunctionContainer = std::unordered_map<std::string, CheckFunction>;

struct CheckClass
{
	CheckVariableContainer vars;
	CheckFunctionContainer methods;
};

using CheckClassContainer = std::unordered_map<std::string, CheckClass>;

std::pair<std::string const, CheckFunction> make_check_function(
	std::string const& name,

	std::vector<TypeContainer> const& params = {},
	TypeContainer const& rtn_types = {}
);

std::pair<std::string const, CheckClass> make_check_class(
	std::string const& name,

	CheckVariableContainer const& vars,
	CheckFunctionContainer const& methods
);