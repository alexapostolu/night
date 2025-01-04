#include "interpreter.h"
#include "bytecode.h"
#include "value.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h> // PRId64
#include <stdbool.h>
#include <assert.h>

Value interpret_bytecodes(byte_t const* codes, int64_t codes_count, Value* variables)
{
	static stack s;
	static int initialized = 0;

	if (!initialized)
	{
		stack_create(&s);
		initialized = 1;
	}

	for (int64_t i = 0; i < codes_count; ++i)
	{
		byte_t const* byte = &codes[i];

		switch (*byte)
		{
		case BytecodeType_S_INT1: interpret_int(&byte, &s, 1, false); break;
		case BytecodeType_S_INT2: interpret_int(&byte, &s, 2, false); break;
		case BytecodeType_S_INT4: interpret_int(&byte, &s, 4, false); break;
		case BytecodeType_S_INT8: interpret_int(&byte, &s, 8, false); break;
		case BytecodeType_U_INT1: interpret_int(&byte, &s, 1, true); break;
		case BytecodeType_U_INT2: interpret_int(&byte, &s, 2, true); break;
		case BytecodeType_U_INT4: interpret_int(&byte, &s, 4, true); break;
		case BytecodeType_U_INT8: interpret_int(&byte, &s, 8, true); break;

		case BytecodeType_FLOAT4: interpret_flt(&byte, &s, 4); break;
		case BytecodeType_FLOAT8: interpret_flt(&byte, &s, 8); break;

		case BytecodeType_NEGATIVE_I: {
			Value* val = stack_top(&s);
			assert(val->is == Value::Int);

			val->as.i *= -1;

			break;
		}
		case BytecodeType_NEGATIVE_F: {
			Value* val = stack_top(&s);
			assert(val->is == Value::Dbl);

			val->as.d *= -1;

			break;
		}

		case BytecodeType_NOT_I: {
			Value* val = stack_top(&s);
			assert(val->is == Value::Int);

			val->as.i = val->as.i ? 0 : 1;
			break;
		}
		case BytecodeType_NOT_F: {
			Value* val = stack_top(&s);
			assert(val->is == Value::Dbl);

			val->as.d = val->as.d ? 0 : 1;
			break;
		}

		case BytecodeType_ADD_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Value::Int);

			val2->as.i += val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_ADD_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Value::Dbl);

			Value* val2 = stack_top(&s);
			assert(val2->is == Value::Dbl);

			val2->as.d += val1->as.d;
			value_destroy(val1);

			break;
		}
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

		case BytecodeType_INDEX_S: interpret_subscript(&byte, &s, true); break;
		case BytecodeType_INDEX_A: interpret_subscript(&byte, &s, false); break;

		case BytecodeType_I2F: {
			Value* val = stack_top(&s);
			assert(val);
			assert(val->is == Value::Int);

			val->is = Value::Dbl;
			val->as.d = (double)val->as.i;

			break;
		}
		case BytecodeType_F2I: {
			Value* val = stack_top(&s);
			assert(val);
			assert(val->is == Value::Dbl);

			val->is = Value::Int;
			val->as.i = (int64_t)val->as.d;

			break;
		}
		case BytecodeType_F2B: {
			Value* val = stack_top(&s);
			assert(val);
			assert(val->is == Value::Int);

			val->is = Value::Dbl;
			val->as.d = (double)val->as.i;

			break;
		}

		case BytecodeType_LOAD: {
			uint64_t id = stack_pop_as_ui(&s);
			stack_push(&s, variables[id]);

			break;
		}
		case BytecodeType_LOAD_ELEM: {
			uint64_t id = stack_pop_as_ui(&s);
			uint64_t dims = stack_pop_as_ui(&s);

			// Iteratively index through the 'dims' dimensions of the array.
			Value* val = variables[id];
			while (dims--)
			{
				uint64_t idx = stack_pop_as_ui(&s);

				assert(val->is == Value::Arr);
				val = val->as.a[idx];
			}

			stack_push(&s, val);

			break;
		}

		case BytecodeType_STORE: {
			Value* val = stack_pop(&s);
			uint64_t id = stack_pop_as_ui(&s);

			variables[id] = val;

			break;
		}

		case BytecodeType_ALLOCATE_STR: interpret_str(&s); break;
		case BytecodeType_ALLOCATE_ARR: interpret_arr(&s); break;
		case BytecodeType_ALLOCATE_ARR_AND_FILL: interpret_arr_fill(&s); break;

		case BytecodeType_STORE_INDEX_A: {
			uint64_t id = stack_pop_as_ui(&s);
			Value* expr = stack_pop(&s);

			Value** val = variables[id];
			while (!stack_is_empty(&s))
			{
				uint64_t idx = stack_pop_as_ui(&s);

				assert(val->is == Value::Arr);
				*val = val->as.a[idx];
			}

			free(*val);
			*val = expr;

			break;
		}
		case BytecodeType_STORE_INDEX_S: {
			uint64_t id = stack_pop_as_ui(&s);
			Value* expr = stack_pop(&s);
			uint64_t idx = stack_pop_as_ui(&s);

			assert(variables[id].is == Value::Str);
			variables[id].as.s[idx] = (char)expr->as.ui;

			break;
		}

		case BytecodeType_JUMP_IF_FALSE: {
			uint64_t offset = stack_pop_as_ui(&s);
			int64_t cond = stack_pop_as_i(&s);

			if (!cond)
				i += offset;

			break;
		}
		case BytecodeType_JUMP: {
			uint64_t offset = stack_pop_as_ui(&s);
			i += offset;

			break;
		}
		case BytecodeType_JUMP_N: {
			uint64_t offset = stack_pop_as_ui(&s);
			i -= offset;

			break;
		}

		case BytecodeType_RETURN: {
			if (stack_is_empty(&s))
				return NULL;

			return stack_pop(&s);
		}

		case BytecodeType_CALL: {
			uint64_t id = stack_pop_as_ui(&s);

			switch (id) {
			case 0: printf(stack_pop_as_i(&s) ? "true" : "false"); break;
			case 1: printf("%c", (char)stack_pop_as_i(&s)); break;
			case 2: printf("%lld", (long long int)stack_pop_as_i(&s)); break;
			case 3: printf("%f", stack_pop_as_d(&s)); break;
			case 4: printf("%s", stack_pop_as_s(&s)); break;
			case 5: interpret_str_input(s); break;
			case 6: break; // char(int); can not remove bytecode because of char(2 + 3) => 2 + 3, and now we have 2, 3, + on the stack that does nothing
			case 7: {
				char* str = stack_pop_as_s(&s);
				int64_t i = stoll(str);
				
				Value* val;
				value_create_i(&val, i);

				stack_push(&s, val);

				break;
			}
			case 8:
				// char is already stored as an int, so int(char) does nothing
				break;
			case 9: {
				int64_t x = stack_pop_as_i(&s);;
				int len = snprintf(NULL, 0, "%" PRId64, x);
				char* str = (char*)malloc(len + 1);
				snprintf(str, len + 1, "%" PRId64, x);

				Value* val;
				value_create_s(&val, str, len);

				stack_push(&s, val);

				break;
			}
			case 10: {
				double x = stack_pop_as_d(&s);
				int len = snprintf(NULL, 0, "%f", x);
				char* str = (char*)malloc(len + 1);
				snprintf(str, len + 1, "%f", x);

				Value* val;
				value_create_s(&val, str, len);

				stack_push(&s, val);

				break;
			}
			case 11: {
				char* str = stack_pop_as_s(&s);
				int64_t i = strlen(str);

				Value* val;
				value_create_i(&val, i);

				stack_push(&s, val);

				break;
			}
			default: {
				Value* ret = interpret_bytecodes(function_codes, function_codes_count, variables);
				if (ret)
					stack_push(&s, ret);

				break;
			}
			}

			break;
		}

		default:
			printf("Unhandled case %d\n", +byte);
			exit(1);
		}
	}

	return NULL;
}

int interpret_int(byte_t** byte, stack* s, byte_t size, bool u)
{
	assert(byte);
	assert(*byte);
	assert(s);

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

int interpret_flt(byte_t** byte, stack* s, byte_t size)
{
	assert(byte);
	assert(*byte);
	assert(s);

	double d;
	memcpy(&d, byte, size);

	Value* val;
	value_create_d(&val, d);
			
	stack_push(&s, val);

	// Iterate to the last byte representing the int, and the calling function
	// interpret_bytecodes will iterate to the next byte sequence.
	while (size--)
		++(*byte);

	return 0;
}

int interpret_str(stack* s)
{
	assert(s);

	uint64_t len = stack_pop_as_ui(s);
	char* str = (char*)malloc((len + 1) * sizeof(char));

	for (uint64_t i = 0; i < len; ++i)
	{
		Value* c = stack_pop(s);
		assert(c->ais == Value::Int);

		str[i] = (char)c->as.i;
		free(c);
	}

	Value* val;
	value_create_s(&val, str, len);

	stack_push(s, val);

	return 0;
}

int interpret_arr(stack* s)
{
	assert(s);

	uint64_t len = stack_pop_as_ui(s);
	Value** arr = (Value**)malloc((len + 1) * sizeof(char));

	for (uint64_t i = 0; i < len; ++i)
		arr[i] = stack_pop(s);

	Value* val;
	value_create_a(&val, arr, len);

	stack_push(s, val);

	return 0;
}

static void fill_arr(Value** arr, uint64_t* dims, uint64_t dims_count, uint64_t curr_dim)
{
	if (curr_dim < dims_count)
	{
		arr->len = dims[curr_dim];
		arr->as.a = (Value**)malloc(arr->len * sizeof(Value**));

		for (size_t i = 0; i < arr->len; ++i)
		{
			Value** elem;
			fill_arr(elem, dims, dims_count, curr_dim + 1);

			arr->as.a[i] = elem;
		}
	}
	else
	{
		memset(&arr.as, 0, sizeof(arr.as));
	}
}

int interpret_arr_fill(stack* s)
{
	uint64_t dims_count = (int64_t)stack_pop_as_ui(s);

	uint64_t* dims = (uint64_t*)malloc(dims_count * sizeof(uint64_t));

	for (uint64_t i = dims_count; i > 0; --i)
		dims[i - 1] = stack_pop_as_ui(s);

	Value** arr;
	fill_arr(arr, dims, dims_count, 0);

	Value* val;
	value_create_a(&val, arr, 0);

	stack_push(s, val);

	return 0;
}

int interpret_subscript(byte_t const** byte, stack* s, bool is_str)
{
	auto container = pop(s);
	auto index = pop(s);

	if (is_string)
		s.emplace((int64_t)container.as.s[index.as.i]);
	else
		s.emplace(container.as.a.data[index.as.i]);
}

int interpret_str_input(stack* s)
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
	
	Value* val;
	value_create_s(&val, buf, len);

	return 0;
}
