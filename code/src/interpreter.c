#include "interpreter.h"
#include "bytecode.h"
#include "value.h"
#include "function.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h> // PRId64
#include <stdbool.h>
#include <assert.h>

Value* interpret_bytecodes(byte_t const* codes, int64_t codes_count, Value** variables, function_t* funcs)
{
	static stack s;
	static int init = 0;

	if (!init)
	{
		stack_create(&s);
		init = 1;
	}

	for (int64_t i = 0; i < codes_count; ++i)
		printf("%d\n", codes[i]);
	printf("\n\n");

	for (byte_t const* byte = &codes[0]; byte < codes + codes_count; ++byte)
	{
		printf("%s\n", byte_to_str(*byte));

		switch (*byte)
		{
		case BytecodeType_U_INT1: interpret_int(&byte, &s, 1); break;
		case BytecodeType_U_INT2: interpret_int(&byte, &s, 2); break;
		case BytecodeType_U_INT4: interpret_int(&byte, &s, 4); break;
		case BytecodeType_U_INT8: interpret_int(&byte, &s, 8); break;

		case BytecodeType_FLOAT4: interpret_flt(&byte, &s, 4); break;
		case BytecodeType_FLOAT8: interpret_flt(&byte, &s, 8); break;

		case BytecodeType_NEGATIVE_I: {
			Value* val = stack_top(&s);
			assert(val->is == Val_Int);

			val->as.i *= -1;

			break;
		}
		case BytecodeType_NEGATIVE_F: {
			Value* val = stack_top(&s);
			assert(val->is == Val_Dbl);

			val->as.d *= -1;

			break;
		}

		case BytecodeType_NOT_I: {
			Value* val = stack_top(&s);
			assert(val->is == Val_Int);

			val->as.i = val->as.i ? 0 : 1;
			break;
		}
		case BytecodeType_NOT_F: {
			Value* val = stack_top(&s);
			assert(val->is == Val_Dbl);

			val->as.d = val->as.d ? 0 : 1;
			break;
		}

		case BytecodeType_ADD_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i += val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_ADD_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val2->is == Val_Dbl);

			val2->as.d += val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_ADD_S: {
			char* s1 = stack_pop_as_s(&s);
			char* s2 = stack_pop_as_s(&s);

			size_t s1_len = strlen(s1);
			size_t s2_len = strlen(s2);
			size_t len = s1_len + s2_len + 1;
			char* result = (char*)malloc(sizeof(char) * len);
			if (!result)
				exit(1);

			strncpy(result, s2, len);
			result[len - 1] = '\0';
			strncat(result, s1, len - s2_len - 1);
			
			Value* new_s;
			value_create_s(&new_s, result, len);

			stack_push(&s, new_s);
			break;
		}

		case BytecodeType_SUB_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i -= val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_SUB_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Dbl);

			val2->as.d -= val1->as.d;
			value_destroy(val1);

			break;
		}

		case BytecodeType_MULT_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val2->is == Val_Int);

			val2->as.i *= val1->as.i;
			value_destroy(val1);
		}
		case BytecodeType_MULT_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val2->is == Val_Dbl);

			val2->as.d *= val1->as.d;
			value_destroy(val1);
		}

		case BytecodeType_DIV_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val2->is == Val_Int);

			val2->as.i /= val1->as.i;
			value_destroy(val1);
		}
		case BytecodeType_DIV_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Dbl);

			val2->as.d /= val1->as.d;
			value_destroy(val1);
		}
		case BytecodeType_MOD_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i %= val1->as.i;
			value_destroy(val1);
		}

		case BytecodeType_LESSER_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i > val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Dbl);

			val2->as.d = val1->as.d > val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Val_Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Val_Str);

			Value* new_val;
			value_create_i(&new_val, strcmp(s1->as.s, s2->as.s) > 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_GREATER_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i < val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Dbl);

			val2->as.d = val1->as.d < val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Val_Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Val_Str);

			Value* new_val;
			value_create_i(&new_val, strcmp(s2->as.s, s1->as.s) > 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_LESSER_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i >= val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_EQUALS_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Dbl);

			val2->as.d = val1->as.d >= val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_LESSER_EQUALS_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Val_Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Val_Str);

			Value* new_val;
			value_create_i(&new_val, strcmp(s1->as.s, s2->as.s) >= 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_GREATER_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i <= val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_EQUALS_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Dbl);

			val2->as.d = val1->as.d <= val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_GREATER_EQUALS_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Val_Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Val_Str);

			Value* new_val;
			value_create_i(&new_val, strcmp(s1->as.s, s2->as.s) <= 0);

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i == val1->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_EQUALS_F: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Dbl);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Dbl);

			val2->as.d = val1->as.d == val1->as.d;
			value_destroy(val1);

			break;
		}
		case BytecodeType_EQUALS_S: {
			Value* s1 = stack_pop(&s);
			assert(s1->is == Val_Str);

			Value* s2 = stack_top(&s);
			assert(s2->is == Val_Str);

			Value* new_val;
			value_create_i(&new_val, !strcmp(s1->as.s, s2->as.s));

			value_destroy(s1);
			value_destroy(s2);

			stack_push(&s, new_val);

			break;
		}

		case BytecodeType_NOT_EQUALS_I: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i != val1->as.i;
			value_destroy(val1);

			break;
		}

		case BytecodeType_AND: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i && val2->as.i;
			value_destroy(val1);

			break;
		}
		case BytecodeType_OR: {
			Value* val1 = stack_pop(&s);
			assert(val1->is == Val_Int);

			Value* val2 = stack_top(&s);
			assert(val1->is == Val_Int);

			val2->as.i = val1->as.i || val1->as.i;
			value_destroy(val1);

			break;
		}

		case ByteType_SUBSCRIPT_S: interpret_subscript_s(&s); break;
		case ByteType_SUBSCRIPT_A: interpret_subscript_a(&s); break;

		case BytecodeType_I2F: {
			Value* val = stack_top(&s);
			assert(val);
			assert(val->is == Val_Int);

			val->is = Val_Dbl;
			val->as.d = (double)val->as.i;

			break;
		}
		case BytecodeType_F2I: {
			Value* val = stack_top(&s);
			assert(val);
			assert(val->is == Val_Dbl);

			val->is = Val_Int;
			val->as.i = (int64_t)val->as.d;

			break;
		}
		case BytecodeType_F2B: {
			Value* val = stack_top(&s);
			assert(val);
			assert(val->is == Val_Int);

			val->is = Val_Dbl;
			val->as.d = (double)val->as.i;

			break;
		}

		case BytecodeType_LOAD: {
			uint64_t id = stack_pop_as_ui(&s);
			stack_push(&s, variables[id]);

			break;
		}
		case BytecodeType_STORE: {
			Value* val = stack_pop(&s);
			uint64_t id = stack_pop_as_ui(&s);

			if (variables[id])
				value_destroy(variables[id]);

			variables[id] = val;

			break;
		}

		case BytecodeType_ALLOCATE_STR: interpret_str(&s); break;
		case BytecodeType_ALLOCATE_ARR: interpret_arr(&s); break;
		case BytecodeType_ALLOCATE_ARR_AND_FILL: interpret_arr_fill(&s); break;

		case ByteType_STORE_SUBSCRIPT_S: {
			Value* expr = stack_pop(&s);
			uint64_t id = stack_pop_as_ui(&s);
			uint64_t idx = stack_pop_as_ui(&s);

			assert(variables[id]->is == Val_Str);
			variables[id]->as.s[idx] = (char)expr->as.ui;

			break;
		}
		case ByteType_STORE_SUBSCRIPT_A: {
			Value* expr = stack_pop(&s);
			uint64_t id = stack_pop_as_ui(&s);

			Value** val = &variables[id];
			while (!stack_is_empty(&s))
			{
				uint64_t idx = stack_pop_as_ui(&s);

				assert((*val)->is == Val_Arr);
				*val = (*val)->as.a[idx];
			}

			free(*val);
			*val = expr;

			break;
		}

		case BytecodeType_JUMP_IF_FALSE: {
			uint64_t offset = stack_pop_as_ui(&s);
			int64_t cond = stack_pop_as_i(&s);

			if (!cond)
				byte += offset;

			break;
		}
		case BytecodeType_JUMP: {
			uint64_t offset = stack_pop_as_ui(&s);
			byte += offset;

			break;
		}
		case BytecodeType_JUMP_N: {
			uint64_t offset = stack_pop_as_ui(&s);
			byte -= offset;

			break;
		}

		case BytecodeType_RETURN:
			if (stack_is_empty(&s))
				return NULL;

			return stack_pop(&s);

		case BytecodeType_CALL: {
			uint64_t id = stack_pop_as_ui(&s);

			switch (id) {
			case 0: printf(stack_pop_as_i(&s) ? "true" : "false"); break;
			case 1: printf("%c", (char)stack_pop_as_i(&s)); break;
			case 2:
				if (stack_top(&s)->is == Val_Int)
					printf("%lld", stack_pop_as_i(&s));
				else
					printf("%lld", stack_pop_as_ui(&s));
				break;
			case 3: printf("%f", stack_pop_as_d(&s)); break;
			case 4: printf("%s", stack_pop_as_s(&s)); break;
			case 5: interpret_str_input(&s); break;
			case 6: break; // char(int); can not remove bytecode because of char(2 + 3) => 2 + 3, and now we have 2, 3, + on the stack that does nothing
			case 7: {
				char* str = stack_pop_as_s(&s);
				int64_t i = strtoll(str, NULL, 10);
				
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
				for (size_t i = 0; i < funcs[id].param_count; ++i)
					variables[funcs[id].param_ids[i]] = stack_pop(&s);

				Value* ret = interpret_bytecodes(funcs[id].bytes, funcs[id].bytes_count, variables, funcs);
				if (ret)
					stack_push(&s, ret);

				break;
			}
			}

			break;
		}

		default:
			printf("Unhandled case %d\n", +(*byte));
			exit(1);
		}
	}

	return NULL;
}

int interpret_int(byte_t const** byte, stack* s, byte_t size)
{
	assert(byte);
	assert(*byte);
	assert(s);

	++(*byte);

	Value* val;

	uint64_t i;
	memcpy(&i, *byte, size);

	value_create_ui(&val, i);

	stack_push(s, val);
	
	// Iterate to the last byte representing the int, and the calling function
	// interpret_bytecodes will iterate to the next byte sequence.
	while (--size)
		++(*byte);

	return 0;
}

int interpret_flt(byte_t const** byte, stack* s, byte_t size)
{
	assert(byte);
	assert(*byte);
	assert(s);

	double d;
	memcpy(&d, byte, size);

	Value* val;
	value_create_d(&val, d);
			
	stack_push(s, val);

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
	assert(str);

	for (uint64_t i = 0; i < len; ++i)
	{
		Value* c = stack_pop(s);
		assert(c->is == Val_Int);

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
	Value** arr = (Value**)malloc((len + 1) * sizeof(Value*));
	assert(arr);

	for (uint64_t i = 0; i < len; ++i)
		arr[i] = stack_pop(s);

	Value* val;
	value_create_a(&val, arr, len);

	stack_push(s, val);

	return 0;
}

static void fill_arr(Value** val, uint64_t* dims, uint64_t dims_count, uint64_t curr_dim)
{
	assert(val);

	*val = (Value*)malloc(sizeof(val));
	assert(*val);

	if (curr_dim < dims_count)
	{
		Value** arr = (Value**)malloc((*val)->len * sizeof(Value*));
		assert(arr);

		for (size_t i = 0; i < dims[curr_dim]; ++i)
		{
			Value* elem;
			fill_arr(&elem, dims, dims_count, curr_dim + 1);

			arr[i] = elem;
		}

		value_create_a(val, arr, (*val)->len);
	}
	else
	{
		memset(&(*val)->as, 0, sizeof((*val)->as));
	}
}

int interpret_arr_fill(stack* s)
{
	uint64_t dims_count = (int64_t)stack_pop_as_ui(s);

	uint64_t* dims = (uint64_t*)malloc(dims_count * sizeof(uint64_t));
	assert(dims);

	for (uint64_t i = dims_count; i > 0; --i)
		dims[i - 1] = stack_pop_as_ui(s);

	Value* val;
	fill_arr(&val, dims, dims_count, 0);

	stack_push(s, val);

	return 0;
}

int interpret_subscript_a(stack* s)
{
	Value* arr = stack_pop(s);
	uint64_t idx = stack_pop_as_ui(s);

	Value* elem = arr->as.a[idx];

	stack_push(s, elem);

	return 0;
}

int interpret_subscript_s(stack* s)
{
	Value* str = stack_pop(s);
	uint64_t idx = stack_pop_as_ui(s);

	uint64_t c = (uint64_t)str->as.s[idx];

	Value* val;
	value_create_ui(&val, c);

	stack_push(s, val);

	return 0;
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
	
	Value* val;
	value_create_s(&val, buf, len);

	return 0;
}
