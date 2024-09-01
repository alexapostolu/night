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
#include <inttypes.h> // PRId64

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes)
{
	std::stack<intpr::Value> s;

	// This freeze is for while loop bytecode.
	// The last JUMP in While loop bytecode jumps to before the start of the vector.
	// But you can not have an iterator point to before the start of a vector. So
	// the iterator will jump to the first element, and stay there instead of being
	// incremented by the for loop.
	bool freeze = false;

	for (auto it = std::begin(codes); it != std::end(codes); ++it)
	{
		if (freeze)
		{
			--it;
			freeze = false;
		}

		switch (*it)
		{
		case BytecodeType_S_INT1:
		case BytecodeType_S_INT2:
		case BytecodeType_S_INT4:
		case BytecodeType_S_INT8:
			s.emplace(get_int<int64_t>(it));
			break;

		case BytecodeType_U_INT1:
		case BytecodeType_U_INT2:
		case BytecodeType_U_INT4:
		case BytecodeType_U_INT8:
			s.emplace(get_int<uint64_t>(it));
			break;

		case BytecodeType_FLOAT4:
		case BytecodeType_FLOAT8:
			s.emplace(get_float(it));
			break;

		case BytecodeType_NEGATIVE_I:
			s.emplace(-pop(s).as.i);
			break;
		case BytecodeType_NEGATIVE_F:
			s.emplace(-pop(s).as.d);
			break;

		case BytecodeType_NOT_I:
			s.emplace((int64_t)!pop(s).as.i);
			break;
		case BytecodeType_NOT_F:
			s.emplace((int64_t)!pop(s).as.d);
			break;

		case BytecodeType_ADD_I:
			s.emplace(pop(s).as.i + pop(s).as.i);
			break;
		case BytecodeType_ADD_F:
			s.emplace(pop(s).as.d + pop(s).as.d);
			break;
		case BytecodeType_ADD_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;

			int s1_len = strlen(s1);
			int s2_len = strlen(s2);
			int len = s1_len + s2_len + 1;
			char* result = (char*)malloc(sizeof(char) * len);
			if (!result)
				exit(1);

			strncpy(result, s2, len);
			result[len - 1] = '\0';
			strncat(result, s1, len - s2_len - 1);

			s.emplace(result);
			break;
		}

		case BytecodeType_SUB_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(-s1 + s2);
			break;
		}
		case BytecodeType_SUB_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace(-s1 + s2);
			break;
		}

		case BytecodeType_MULT_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(s1 * s2);
			break;
		}
		case BytecodeType_MULT_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace(s1 * s2);
			break;
		}

		case BytecodeType_DIV_I: {
			auto s2 = pop(s);
			s.emplace(pop(s).as.i / s2.as.i);
			break;
		}
		case BytecodeType_DIV_F: {
			auto s2 = pop(s);
			s.emplace(pop(s).as.d / s2.as.d);
			break;
		}
		case BytecodeType_MOD_I: {
			auto s2 = pop(s);
			s.emplace(pop(s).as.i % s2.as.i);
			break;
		}

		// stack values are in opposite order, so we switch signs to account for that
		case BytecodeType_LESSER_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(int64_t(s1 > s2));
			break;
		}
		case BytecodeType_LESSER_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace(int64_t(s1 > s2));
			break;
		}
		case BytecodeType_LESSER_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;
			s.emplace(int64_t(strcmp(s1, s2) > 0));
			break;
		}

		case BytecodeType_GREATER_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(int64_t(s1 < s2));
			break;
		}
		case BytecodeType_GREATER_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace(int64_t(s1 < s2));
			break;
		}
		case BytecodeType_GREATER_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;
			s.emplace(int64_t(strcmp(s1, s2) < 0));
			break;
		}

		case BytecodeType_LESSER_EQUALS_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace((int64_t)(s1 >= s2));
			break;
		}
		case BytecodeType_LESSER_EQUALS_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace((int64_t)(s1 >= s2));
			break;
		}
		case BytecodeType_LESSER_EQUALS_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;
			s.emplace((int64_t)(strcmp(s1, s2) >= 0));
			break;
		}

		case BytecodeType_GREATER_EQUALS_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace((int64_t)(s1 <= s2));
			break;
		}
		case BytecodeType_GREATER_EQUALS_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace((int64_t)(s1 <= s2));
			break;
		}
		case BytecodeType_GREATER_EQUALS_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;
			s.emplace((int64_t)(strcmp(s1, s2) <= 0));
			break;
		}

		case BytecodeType_EQUALS_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(int64_t(s1 == s2));
			break;
		}
		case BytecodeType_EQUALS_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace(int64_t(s1 == s2));
			break;
		}
		case BytecodeType_EQUALS_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;
			s.emplace(int64_t(!strcmp(s1, s2)));
			break;
		}

		case BytecodeType_NOT_EQUALS_I: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(int64_t(s1 != s2));
			break;
		}

		case BytecodeType_AND: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(int64_t(s1 && s2));
			break;
		}
		case BytecodeType_OR: {
			auto s1 = pop(s).as.i;
			auto s2 = pop(s).as.i;
			s.emplace(int64_t(s1 || s2));
			break;
		}

		case BytecodeType_INDEX_S: push_subscript(s, true); break;
		case BytecodeType_INDEX_A: push_subscript(s, false); break;

		case BytecodeType_I2F:
			s.emplace(float(pop(s).as.i));
			break;
		case BytecodeType_F2I:
			s.emplace(int64_t(pop(s).as.d));
			break;
		case BytecodeType_F2B:
			s.emplace((int64_t)(pop(s).as.d != 0));
			break;

		case BytecodeType_LOAD:
			s.emplace(scope.vars[pop(s).as.i]);
			break;

		case BytecodeType_STORE: {
			auto id = pop(s);
			scope.vars[id.as.i] = pop(s);

			break;
		}

		case BytecodeType_LOAD_ELEM: {
			auto id = pop(s).as.i;
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

		case BytecodeType_ALLOCATE_STR: push_str(s); break;
		case BytecodeType_ALLOCATE_ARR: push_arr(s); break;
		case BytecodeType_ALLOCATE_ARR_AND_FILL: push_arr_and_fill(s); break;

		case BytecodeType_STORE_INDEX_A: {
			auto id = pop(s).as.i;
			auto expr = pop(s);
			intpr::Value* val = &scope.vars[id];
			while (!s.empty())
			{
				auto i = pop(s).as.i;
				val = &val->as.a.data[i];
			}
			*val = expr;
			break;
		}

		case BytecodeType_STORE_INDEX_S: {
			auto id = pop(s).as.i;
			auto expr = pop(s);
			scope.vars[id].as.s[pop(s).as.i] = (char)expr.as.i;
			break;
		}

		case BytecodeType_JUMP_IF_FALSE: {
			auto offset = pop(s).as.i;
			if (!pop(s).as.i)
				std::advance(it, offset);
			break;
		}

		case BytecodeType_JUMP:
			std::advance(it, pop(s).as.i);
			break;
		case BytecodeType_JUMP_N:
			std::advance(it, -pop(s).as.i);
			freeze = true;
			break;

		case BytecodeType_RETURN:
			if (s.empty())
				return std::optional<intpr::Value>(std::nullopt);
			return pop(s);

		case BytecodeType_CALL: {
			auto id = pop(s).as.i;

			switch (id)
			{
			case 0: std::cout << (pop(s).as.i ? "true" : "false"); break;
			case 1: std::cout << (char)pop(s).as.i; break;
			case 2: std::cout << pop(s).as.i; break;
			case 3: std::cout << pop(s).as.d; break;
			case 4: printf("%s", pop(s).as.s); break;
			case 5: push_string_input(s); break;
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
				int length = snprintf(NULL, 0, "%" PRId64, x);
				char* str = (char*)malloc(length + 1);
				snprintf(str, length + 1, "%" PRId64, x);

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

double get_float(bytecodes_t::const_iterator& it)
{
	assert(*it == BytecodeType_FLOAT4 || *it == BytecodeType_FLOAT8);

	if (*it == BytecodeType_FLOAT4)
	{
		bytecode_t float_codes[4];
		for (int i = 0; i < sizeof(float_codes) / sizeof(float_codes[0]); ++i)
			float_codes[i] = *(++it);

		float f;
		std::memcpy(&f, float_codes, sizeof(float));

		return (double)f;
	}
	else if (*it == BytecodeType_FLOAT8)
	{
		bytecode_t double_codes[8];
		for (int i = 0; i < sizeof(double_codes) / sizeof(double_codes[0]); ++i)
			double_codes[i] = *(++it);

		double d;
		std::memcpy(&d, double_codes, sizeof(float));

		return d;
	}
	else
	{
		exit(1);
	}
}

void push_str(std::stack<intpr::Value>& s)
{
	auto size = pop(s).as.i;
	char* arr = new char[size + 1];

	for (auto i = 0; i < size; ++i)
		arr[i] = (char)pop(s).as.i;

	arr[size] = '\0';

	s.push(arr);
}

void push_arr(std::stack<intpr::Value>& s)
{
	int size = (int)pop(s).as.i;
	intpr::Value arr;
	arr.as.a.size = size;
	arr.as.a.data = new intpr::Value[size];

	for (auto i = 0; i < size; ++i)
		arr.as.a.data[i] = pop(s);

	s.push(arr);
}

void push_arr_and_fill(std::stack<intpr::Value>& s)
{
	int dimensions = (int)pop(s).as.i;
	std::vector<int> sizes(dimensions);

	for (int i = dimensions - 1; i >= 0; --i)
		sizes[i] = (int)pop(s).as.i;

	intpr::Value arr;
	fill_arr(arr, s, sizes, 0);

	s.push(arr);
}

void fill_arr(intpr::Value& arr, std::stack<intpr::Value>& s, std::vector<int> const& dimensions, int current_dimension)
{
	if (current_dimension < dimensions.size())
	{
		arr.as.a.size = dimensions[current_dimension];
		arr.as.a.data = new intpr::Value[dimensions[current_dimension]];

		for (int i = 0; i < dimensions[current_dimension]; ++i)
		{
			intpr::Value element;
			fill_arr(element, s, dimensions, current_dimension + 1);

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

void push_string_input(std::stack<intpr::Value>& s)
{
	int size = 32;
	char* buf = (char*)malloc(sizeof(char) * size);

	if (!buf)
		exit(1);

	int c;
	int len = 0;
	while ((c = getchar()) != '\n' && c != EOF)
	{
		if (len == size - 1)
		{
			size *= 1.5;
			char* new_buf = (char*)realloc(buf, sizeof(char) * size);

			if (!new_buf)
			{
				free(buf);
				exit(1);
			}

			buf = new_buf;
		}

		buf[len] = c;
		++len;
	}

	buf[len] = '\0';
	s.emplace(buf);
}

intpr::Value pop(std::stack<intpr::Value>& s)
{
	assert(!s.empty());

	auto val = s.top();
	s.pop();

	return val;
}