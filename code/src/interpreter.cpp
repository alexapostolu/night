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
	// Disable stdout buffering
	setbuf(stdout, NULL);

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
		case ByteType_sINT1: s.emplace(interpret_int<int64_t>(it, 1)); break;
		case ByteType_sINT2: s.emplace(interpret_int<int64_t>(it, 2)); break;
		case ByteType_sINT4: s.emplace(interpret_int<int64_t>(it, 4)); break;
		case ByteType_sINT8: s.emplace(interpret_int<int64_t>(it, 8)); break;

		case ByteType_uINT1: s.emplace(interpret_int<uint64_t>(it, 1)); break;
		case ByteType_uINT2: s.emplace(interpret_int<uint64_t>(it, 2)); break;
		case ByteType_uINT4: s.emplace(interpret_int<uint64_t>(it, 4)); break;
		case ByteType_uINT8: s.emplace(interpret_int<uint64_t>(it, 8)); break;

		case ByteType_FLT4: s.emplace(interpret_flt(it, 4)); break;
		case ByteType_FLT8: s.emplace(interpret_flt(it, 8)); break;

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

			size_t s1_len = strlen(s1);
			size_t s2_len = strlen(s2);
			size_t len = s1_len + s2_len + 1;
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
		case ByteType_MOD: {
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
		case BytecodeType_NOT_EQUALS_F: {
			auto s1 = pop(s).as.d;
			auto s2 = pop(s).as.d;
			s.emplace(int64_t(s1 != s2));
			break;
		}
		case BytecodeType_NOT_EQUALS_S: {
			auto s1 = pop(s).as.s;
			auto s2 = pop(s).as.s;
			s.emplace((int64_t)strcmp(s1, s2));
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

		case BytecodeType_I2F: s.emplace(float(pop(s).as.i)); break;
		case BytecodeType_F2I: s.emplace(int64_t(pop(s).as.d)); break;

		case ByteType_LOAD: {
			uint64_t id = pop(s).as.ui;
			s.emplace(scope.vars[id]);

			break;
		}

		case ByteType_STORE: {
			uint64_t id = pop(s).as.ui;
			scope.vars[id] = pop(s);

			break;
		}

		case BytecodeType_LOAD_ELEM: {
			uint64_t id = pop(s).as.ui;
			uint64_t num = pop(s).as.ui;
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
		case ByteType_JUMP_N:
			std::advance(it, -pop(s).as.i);
			freeze = true;
			break;

		case BytecodeType_RETURN: {
			if (s.empty())
				return std::nullopt;

			return pop(s);
		}

		case BytecodeType_CALL: {
			int64_t id = pop(s).as.i;

			switch (id)
			{
			case 0: printf(pop(s).as.i ? "true" : "false"); break;
			case 1: printf("%c", (char)pop(s).as.i); break;
			case 2: printf("%lld", (long long int)pop(s).as.i); break;
			case 3: printf("%.17gf", pop(s).as.d); break;
			case 4: printf("%s", pop(s).as.s); break;
			case 5: push_string_input(s); break;
			case 6: break; // char(int); can not remove bytecode because of char(2 + 3) => 2 + 3, and now we have 2, 3, + on the stack that does nothing
			case 7: {
				char* str = pop(s).as.s;
				try {
					s.emplace((int64_t)std::stoll(str));
				}
				catch (std::invalid_argument&) {
					printf("Cannot convert string '%s' to integral type.\n", str);
					exit(1);
				}
				break;
			}
			case 8: {
				// char is already stored as an int, so int(char) does nothing
				break;
			}
			case 9: {
				int64_t x = pop(s).as.i;
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
				auto x = pop(s).as.i;
				char* str = (char*)malloc(2);
				str[0] = x;

				s.emplace(str);
				break;
			}
			case 12: {
				s.emplace((int64_t)strlen(pop(s).as.s));
				break;
			}
			default: {
				InterpreterScope func_scope{ scope.vars };

				for (int i = scope.funcs[id].param_ids.size() - 1; i >= 0; --i)
					func_scope.vars[InterpreterScope::funcs[id].param_ids[i]] = pop(s);

				auto rtn_value = interpret_bytecodes(func_scope, InterpreterScope::funcs[id].codes);
				if (rtn_value.has_value())
					s.push(*rtn_value);

				// Update parent scope's variables to any changes made in child scope.
				for (auto const& [id, val] : func_scope.vars)
				{
					if (scope.vars.contains(id))
						scope.vars[id] = val;
				}

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

double interpret_flt(bytecodes_t::const_iterator& it, unsigned short size)
{
	assert(size == 4 || size == 8);

	union {
		uint8_t bytes[sizeof(double)];
		double d;
		float f;
	};

	for (unsigned short i = 0; i < size; ++i)
		bytes[i] = *(++it);

	return size == 4 ? f : d;
}

void push_str(std::stack<intpr::Value>& s)
{
	uint64_t size = pop(s).as.ui;
	char* arr = new char[size + 1];

	for (uint64_t i = 0; i < size; ++i)
		arr[i] = (char)pop(s).as.i;

	arr[size] = '\0';

	s.push(arr);
}

void push_arr(std::stack<intpr::Value>& s)
{
	uint64_t size = pop(s).as.ui;
	intpr::Value arr;
	arr.as.a.size = size;
	arr.as.a.data = new intpr::Value[size];

	for (uint64_t i = 0; i < size; ++i)
		arr.as.a.data[i] = pop(s);

	s.push(arr);
}

void push_arr_and_fill(std::stack<intpr::Value>& s)
{
	uint64_t dimensions = pop(s).as.ui;
	std::vector<int> sizes(dimensions);

	for (uint64_t i = dimensions; i > 0; --i)
		sizes[i - 1] = (int)pop(s).as.i;
	
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
			size = (int)(size * 1.5);
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