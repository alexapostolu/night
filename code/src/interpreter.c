#include "interpreter.h"
#include "bytecode.h"
#include "value.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h> // PRId64
#include <assert.h>

Value interpret_bytecodes(byte_t const* codes, size_t codes_count, Value* variables)
{
	static stack s;
	static int initialized = 0;

	if (!initialized)
	{
		stack_create(&s);
		initialized = 1;
	}

	// This freeze is for while loop bytecode.
	// The last JUMP in While loop bytecode jumps to before the start of the vector.
	// But you can not have an iterator point to before the start of a vector. So
	// the iterator will jump to the first element, and stay there instead of being
	// incremented by the for loop.
	int freeze = 0;

	for (size_t i = 0; i < codes_count; ++i)
	{
		if (freeze)
		{
			--it;
			freeze = 0;
		}

		byte_t const* byte = &codes[i];

		switch (*byte)
		{
		case BytecodeType_S_INT1:
			interpret_int(&byte, &s, 1, 0);
			break;
		case BytecodeType_S_INT2:
			interpret_int(&byte, &s, 2, 0);
			break;
		case BytecodeType_S_INT4:
			interpret_int(&byte, &s, 4, 0);
			break;
		case BytecodeType_S_INT8:
			interpret_int(&byte, &s, 8, 0);
			break;
		case BytecodeType_U_INT1:
			interpret_int(&byte, &s, 1, 1);
			break;
		case BytecodeType_U_INT2:
			interpret_int(&byte, &s, 2, 1);
			break;
		case BytecodeType_U_INT4:
			interpret_int(&byte, &s, 4, 1);
			break;
		case BytecodeType_U_INT8:
			interpret_int(&byte, &s, 8, 1);
			break;

		case BytecodeType_FLOAT4:
		case BytecodeType_FLOAT8:
			double f = get_float(codes, i);

			Value* val;
			value_create_d(&val, f);
			
			stack_push(&s, val);

			break;

		case BytecodeType_NEGATIVE_I:
			Value* val = stack_top(&s);
			assert(val->is == Value::Int);

			val->as.i *= -1;

			break;
		case BytecodeType_NEGATIVE_F:
			Value* val = stack_top(&s);
			assert(val->is == Value::Dbl);

			val->as.d *= -1;

			break;

		case BytecodeType_NOT_I:
			Value* val = stack_top(&s);
			assert(val->is == Value::Int);

			val->as.i = val->as.i ? 0 : 1;
			break;

		case BytecodeType_NOT_F:
			Value* val = stack_top(&s);
			assert(val->is == Value::Dbl);

			val->as.d = val->as.d ? 0 : 1;
			break;

		case BytecodeType_ADD_I:
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i += val1->as.i;
			value_destroy(val1);

			break;
		case BytecodeType_ADD_F:
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val2->is == Value::Dbl);

			val2->as.d += val1->as.d;
			value_destroy(val1);

			break;
		case BytecodeType_ADD_S: {
			Value* s1 = stack_pop(&s);
			Value* s2 = stack_pop(&s);

			int s1_len = strlen(s1->as.s);
			int s2_len = strlen(s2->as.s);
			int len = s1_len + s2_len + 1;
			char* result = (char*)malloc(sizeof(char) * len);
			if (!result)
				exit(1);

			strncpy(result, s2, len);
			result[len - 1] = '\0';
			strncat(result, s1, len - s2_len - 1);

			value_destroy(s1);
			value_destroy(s2);
			
			Value* new_s;
			value_create_s(&new_s, result, len);

			stack_push(&s, new_s);
			break;
		}

		case BytecodeType_SUB_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i -= val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_SUB_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Dbl);

			val2->as.d -= val1->as.d;
			value_destroy(val1);

			break;
		}

		case BytecodeType_MULT_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val2->is == Value::Int);

			val2->as.i *= val1->as.i;
			value_destroy(val1);
		}
		case BytecodeType_MULT_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val2->is == Value::Dbl);

			val2->as.d *= val1->as.d;
			value_destroy(val1);
		}

		case BytecodeType_DIV_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val2->is == Value::Int);

			val2->as.i /= val1->as.i;
			value_destroy(val1);
		}
		case BytecodeType_DIV_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Dbl);

			val2->as.d /= val1->as.d;
			value_destroy(val1);
		}
		case BytecodeType_MOD_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i %= val1->as.i;
			value_destroy(val1);
		}

		// stack values are in opposite order, so we switch signs to account for that
		case BytecodeType_LESSER_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i > val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Dbl);

			val2->as.f = val1->as.f > val1->as.f;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Value::Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Value::Str);

			Value* new_val;
			value_create_i(&val, strcmp(s1->as.s, s2->as.s) > 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_GREATER_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i < val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Dbl);

			val2->as.d = val1->as.d < val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Value::Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Value::Str);

			Value* new_val;
			value_create_i(&val, strcmp(s2->as.s, s1->as.s) > 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_LESSER_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i >= val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_EQUALS_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Dbl);

			val2->as.f = val1->as.f >= val1->as.f;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_EQUALS_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Value::Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Value::Str);

			Value* new_val;
			value_create_i(&val, strcmp(s1->as.s, s2->as.s) >= 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_GREATER_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i <= val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_EQUALS_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Dbl);

			val2->as.d = val1->as.d <= val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_EQUALS_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Value::Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Value::Str);

			Value* new_val;
			value_create_i(&val, strcmp(s1->as.s, s2->as.s) <= 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i == val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_EQUALS_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Dbl);

			val2->as.d = val1->as.d == val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_EQUALS_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Value::Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Value::Str);

			Value* new_val;
			value_create_i(&val, !strcmp(s1->as.s, s2->as.s));

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_NOT_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i != val1->as.i;
			value_destroy(val1);

			break;
		}

		case BytecodeType_AND: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i && val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_OR: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i = val1->as.i || val1->as.i;
			value_destroy(val1);

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
			auto val = pop(s);
			auto id = pop(s);
			scope.vars[id.as.i] = val;

			break;
		}

		case BytecodeType_LOAD_ELEM: {
			auto id = pop(s).as.i;
			auto num = pop(s).as.i;
			Value* val = &scope.vars[id];
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
			if (stack_is_empty(&s))
				return NULL;

			return stack_pop(&s);

		case BytecodeType_CALL: {
			int64_t id = pop(s).as.i;

			switch (id)
			{
			case 0: printf(pop(s).as.i ? "true" : "false"); break;
			case 1: printf("%c", (char)pop(s).as.i); break;
			case 2: printf("%lld", (long long int)pop(s).as.i); break;
			case 3: printf("%f", pop(s).as.d); break;
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
				int64_t x = pop(s).as.i;
				int length = snprintf(NULL, 0, "%" PRId64, x);
				char* str = (char*)malloc(length + 1);
				snprintf(str, length + 1, "%" PRId64, x);

				s.emplace(str, length);
				break;
			}
			case 10: {
				auto x = pop(s).as.d;
				int length = snprintf(NULL, 0, "%f", x);
				char* str = (char*)malloc(length + 1);
				snprintf(str, length + 1, "%f", x);

				s.emplace(str, length);
				break;
			}
			case 11: {
				s.emplace((int64_t)strlen(pop(s).as.s));
				break;
			}
			default: {
				Value* function_vars = variables;

				auto rtn_value = interpret_bytecodes(InterpreterScope::funcs[id].codes, function_vars);
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

	return NULL;
}

int interpret_int(
	byte_t** byte,
	stack* s,
	byte_t size,
	int u)
{
	Value* val;

	if (u)
	{
		uint64_t i;
		memcpy(&i, byte, size);

		value_create_ui(&val, i);
	}
	else
	{
		int64_t i;
		memcpy(&i, byte, size);

		value_create_i(&val, i);
	}

	stack_push(&s, val);
	
	// Iterate to the last byte representing the int, and the calling function
	// interpret_bytecodes will iterate to the next byte sequence.
	while (size--)
		++(*byte);

	return 0;
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
	char* str = new char[size + 1];

	for (auto i = 0; i < size; ++i)
		str[i] = (char)pop(s).as.i;

	str[size] = '\0';

	s.emplace(str, size);
}

void push_arr(std::stack<Value>& s)
{
	int size = (int)pop(s).as.i;
	Value arr;
	arr.as.a.size = size;
	arr.as.a.data = new intpr::Value[size];

	for (auto i = 0; i < size; ++i)
		arr.as.a.data[i] = pop(s);

	s.push(arr);
}

void push_arr_and_fill(std::stack<Value>& s)
{
	int dimensions = (int)pop(s).as.i;
	std::vector<int> sizes(dimensions);

	for (int i = dimensions - 1; i >= 0; --i)
		sizes[i] = (int)pop(s).as.i;

	Value arr;
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
	s.emplace(buf, len);
}

intpr::Value pop(std::stack<intpr::Value>& s)
{
	assert(!s.empty());

	auto val = s.top();
	s.pop();

	return val;
}