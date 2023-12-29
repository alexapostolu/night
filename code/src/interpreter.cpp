#include "interpreter.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <iostream>
#include <cmath>
#include <stack>
#include <optional>
#include <cstring>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes)
{
	std::stack<intpr::Value> s;

	for (auto it = std::begin(codes); it != std::end(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::S_INT1:
		case BytecodeType::S_INT2:
		case BytecodeType::S_INT4:
		case BytecodeType::S_INT8:
			s.emplace(get_int<int64_t>(it));
			break;

		case BytecodeType::U_INT1:
		case BytecodeType::U_INT2:
		case BytecodeType::U_INT4:
		case BytecodeType::U_INT8:
			s.emplace(get_int<uint64_t>(it));
			break;

		case BytecodeType::FLOAT4:
		case BytecodeType::FLOAT8:
			push_float(s, it);
			break;

		case BytecodeType::NEGATIVE_I:
			s.emplace(-pop(s).as.i);
			break;
		case BytecodeType::NEGATIVE_F:
			s.emplace(-pop(s).as.d);
			break;

		case BytecodeType::NOT_I:
			s.emplace((int64_t)!pop(s).as.i);
			break;
		case BytecodeType::NOT_F:
			s.emplace((int64_t)!pop(s).as.d);
			break;

		case BytecodeType::ADD_I:
			s.emplace(pop(s).as.i + pop(s).as.i);
			break;
		case BytecodeType::ADD_F:
			s.emplace(pop(s).as.d + pop(s).as.d);
			break;
		case BytecodeType::ADD_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;

			char* result = (char*)malloc(strlen(s1) + strlen(s2) + 1);
			if (!result)
				throw night::create_runtime_error("could not allocate memory");

			strcpy(result, s2);
			strcat(result, s1);

			s.emplace(result);
			break;
		}

		case BytecodeType::SUB_I:
			s.emplace(-pop(s).as.i + pop(s).as.i);
			break;
		case BytecodeType::SUB_F:
			s.emplace(-pop(s).as.d + pop(s).as.d);
			break;

		case BytecodeType::MULT_I:
			s.emplace(pop(s).as.i * pop(s).as.i);
			break;
		case BytecodeType::MULT_F:
			s.emplace(pop(s).as.d * pop(s).as.d);
			break;

		case BytecodeType::DIV_I: {
			auto s2 = pop(s);
			s.emplace(pop(s).as.i / s2.as.i);
			break;
		}
		case BytecodeType::DIV_F: {
			auto s2 = pop(s);
			s.emplace(pop(s).as.d / s2.as.d);
			break;
		}
		case BytecodeType::MOD_I: {
			auto s2 = pop(s);
			s.emplace(pop(s).as.i % s2.as.i);
			break;
		}

		// stack values are in opposite order, so we switch signs to account for that
		case BytecodeType::LESSER_I:
			s.emplace(int64_t(pop(s).as.i > pop(s).as.i));
			break;
		case BytecodeType::LESSER_F:
			s.emplace(int64_t(pop(s).as.d > pop(s).as.d));
			break;
		case BytecodeType::LESSER_S:
			s.emplace(int64_t(strcmp(pop(s).as.s, pop(s).as.s) > 0));
			break;

		case BytecodeType::GREATER_I:
			s.emplace(int64_t(pop(s).as.i < pop(s).as.i));
			break;
		case BytecodeType::GREATER_F:
			s.emplace(int64_t(pop(s).as.d < pop(s).as.d));
			break;
		case BytecodeType::GREATER_S:
			s.emplace(int64_t(strcmp(pop(s).as.s, pop(s).as.s) < 0));
			break;

		case BytecodeType::LESSER_EQUALS_I:
			s.emplace((int64_t)(pop(s).as.i >= pop(s).as.i));
			break;
		case BytecodeType::LESSER_EQUALS_F:
			s.emplace((int64_t)(pop(s).as.d >= pop(s).as.d));
			break;
		case BytecodeType::LESSER_EQUALS_S:
			s.emplace((int64_t)(strcmp(pop(s).as.s, pop(s).as.s) >= 0));
			break;

		case BytecodeType::GREATER_EQUALS_I:
			s.emplace((int64_t)(pop(s).as.i <= pop(s).as.i));
			break;
		case BytecodeType::GREATER_EQUALS_F:
			s.emplace((int64_t)(pop(s).as.d <= pop(s).as.d));
			break;
		case BytecodeType::GREATER_EQUALS_S:
			s.emplace((int64_t)(strcmp(pop(s).as.s, pop(s).as.s) <= 0));
			break;

		case BytecodeType::EQUALS_I:
			s.emplace(int64_t(pop(s).as.i == pop(s).as.i));
			break;
		case BytecodeType::EQUALS_F:
			s.emplace(int64_t(pop(s).as.d == pop(s).as.d));
			break;
		case BytecodeType::EQUALS_S:
			s.emplace(int64_t(!strcmp(pop(s).as.s, pop(s).as.s)));
			break;

		case BytecodeType::AND:
			s.emplace(int64_t(pop(s).as.i && pop(s).as.i));
			break;
		case BytecodeType::OR:
			s.emplace(int64_t(pop(s).as.i || pop(s).as.i));
			break;

		case BytecodeType::INDEX_S: push_subscript(s, true); break;
		case BytecodeType::INDEX_A: push_subscript(s, false); break;

		case BytecodeType::I2F:
			s.emplace(float(pop(s).as.i));
			break;
		case BytecodeType::F2I:
			s.emplace(int64_t(pop(s).as.d));
			break;
		case BytecodeType::F2B:
			s.emplace((int64_t)(pop(s).as.d != 0));
			break;

		case BytecodeType::LOAD:
			s.emplace(scope.vars[*(++it)]);
			break;

		case BytecodeType::STORE: {
			auto id = pop(s);
			scope.vars[id.as.i] = pop(s);

			break;
		}

		case BytecodeType::LOAD_ELEM: {
			auto id = *(++it);
			auto num = pop(s).as.i;
			intpr::Value* val = &scope.vars[id];
			while (num--)
			{
				auto i = pop(s).as.i;
				val = &val->as.a.data[i];
			}
			s.push(*val);
			break;
		}

		case BytecodeType::ALLOCATE_STR: push_str(s); break;
		case BytecodeType::ALLOCATE_ARR: push_arr(s); break;
		case BytecodeType::ALLOCATE_ARR_AND_FILL: push_arr_and_fill(s); break;

		case BytecodeType::STORE_INDEX: {
			auto expr = pop(s);
			auto id = *(++it);
			intpr::Value* val = &scope.vars[id];
			while (!s.empty())
			{
				auto i = pop(s).as.i;
				val = &val->as.a.data[i];
			}
			*val = expr;
			break;
		}

		case BytecodeType::JUMP_IF_FALSE: {
			auto offset = pop(s).as.i;
			if (!pop(s).as.i)
				std::advance(it, offset);
			break;
		}

		case BytecodeType::JUMP:
			std::advance(it, *(++it));
			break;
		case BytecodeType::NJUMP:
			std::advance(it, -(*(++it)));
			break;

		case BytecodeType::RETURN:
			if (s.empty())
				return std::optional<intpr::Value>(std::nullopt);
			return pop(s);

		case BytecodeType::CALL: {
			auto id = *(++it);

			switch (id)
			{
			case 0: std::cout << (pop(s).as.i ? "true" : "false"); break;
			case 1: std::cout << (char)pop(s).as.i; break;
			case 2: std::cout << pop(s).as.i; break;
			case 3: std::cout << pop(s).as.d; break;
			case 4: printf("%s", pop(s).as.s); break;
			case 5: { char* str = new char[100]; scanf("%s", str); s.emplace(str); }; break;
			case 6: break; // char(int); can not remove bytecode because of char(2 + 3) => 2 + 3, and now we have 2, 3, + on the stack that does nothing
			case 7: {
				s.emplace((int64_t)std::stoll(pop(s).as.s));
				break;
			}
			case 8: {
				// char is already stored as an int, so int(char) does nothing
				break;
			}
			case 9: {
				auto x = pop(s).as.i;
				int length = snprintf(NULL, 0, "%lld", x);
				char* str = (char*)malloc(length + 1);
				snprintf(str, length + 1, "%lld", x);

				s.emplace(str);
				break;
			}
			case 10: {
				auto x = pop(s).as.d;
				int length = snprintf(NULL, 0, "%f", x);
				char* str = (char*)malloc(length + 1);
				snprintf(str, length + 1, "%f", x);

				s.emplace(str);
				break;
			}
			case 11: {
				s.emplace((int64_t)strlen(pop(s).as.s));
				break;
			}
			default: {
				InterpreterScope func_scope{ scope.vars };

				for (int i = 0; i < scope.funcs[id].param_ids.size(); ++i)
					func_scope.vars[InterpreterScope::funcs[id].param_ids[i]] = pop(s);

				auto rtn_value = interpret_bytecodes(func_scope, InterpreterScope::funcs[id].codes);
				if (rtn_value.has_value())
					s.push(*rtn_value);

				break;
			}
			}

			break;
		}

		default:
			throw debug::unhandled_case(*it);
		}
	}

	return std::nullopt;
}

void push_float(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	int count;
	switch ((BytecodeType)(*it))
	{
	case BytecodeType::FLOAT4: count = 4; break;
	case BytecodeType::FLOAT8: count = 8; break;
	default: throw debug::unhandled_case(*it);
	}

	float f;
	++it;
	std::memcpy(&f, &(*it), sizeof(float));

	s.emplace((double)f);

	// -1 to account for the advancement in the main for loop
	std::advance(it, count - 1);
}

void push_str(std::stack<intpr::Value>& s)
{
	int size = pop(s).as.i;
	char* arr = new char[size + 1];

	for (auto i = 0; i < size; ++i)
		arr[i] = pop(s).as.i;

	arr[size] = '\0';

	s.push(arr);
}

void push_arr(std::stack<intpr::Value>& s)
{
	int size = pop(s).as.i;
	intpr::Value arr;
	arr.as.a.size = size;
	arr.as.a.data = new intpr::Value[size];

	for (auto i = 0; i < size; ++i)
		arr.as.a.data[i] = pop(s);

	s.push(arr);
}

void push_arr_and_fill(std::stack<intpr::Value>& s)
{
	auto sizes = pop(s).as.i;
	intpr::Value arr;

	fill_arr(arr, s, sizes, 0);
}

void fill_arr(intpr::Value& arr, std::stack<intpr::Value>& s, int size, int count)
{
	if (count < size)
	{
		auto size = pop(s).as.i;

		arr.as.a.size = size;
		arr.as.a.data = new intpr::Value[size];

		for (int i = 0; i < size; ++i)
		{
			intpr::Value element;
			fill_arr(element, s, size, count + 1);

			arr.as.a.data[i] = element;
		}
	}
	else
	{
		arr.as.i = 0;
		arr.as.ui = 0;
		arr.as.d = 0;
	}
}

void push_subscript(std::stack<intpr::Value>& s, bool is_string)
{
	auto container = pop(s);
	auto index = pop(s);

	if (is_string)
		s.emplace((int64_t)container.as.s[index.as.i]);
	else
		s.emplace(container.as.a.data[index.as.i]);
}

intpr::Value pop(std::stack<intpr::Value>& s)
{
	assert(!s.empty());

	auto val = s.top();
	s.pop();

	return val;
}