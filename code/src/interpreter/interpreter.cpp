#include "interpreter/interpreter.hpp"
#include "interpreter/interpreter_scope.hpp"
#include "common/error.hpp"
#include "common/debug.hpp"
#include "language.hpp"

#include <iostream>
#include <cmath>
#include <stack>
#include <optional>
#include <cstring>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <inttypes.h> // PRId64

static int64_t str_to_int(char* s)
{
	assert(s);

	errno = 0;
	char* temp;
	int64_t val = strtoll(s, &temp, 10);

	if (temp == s || *temp != '\0' ||
		((val == LLONG_MIN || val == LLONG_MAX) && errno == ERANGE))
		fprintf(stderr, "Could not convert '%s' to long long and leftover string is: '%s'\n", s, temp);

	return val;
}

static uint64_t str_to_uint(char* s)
{
	assert(s);

	errno = 0;
	char* temp;
	uint64_t val = strtoull(s, &temp, 10);

	if (temp == s || *temp != '\0' ||
		(val == ULLONG_MAX && errno == ERANGE))
		fprintf(stderr, "Error: Could not convert '%s' to uint64_t. Leftover string: '%s'. "
			"Value is %llu and errno is %d.\n", s, temp, val, errno);

	return val;
}

static float str_to_float(char* s)
{
	assert(s);

	errno = 0;
	char* temp;
	float val = strtod(s, &temp);

	if (temp == s || *temp != '\0' ||
		((val == FLT_MIN || val == FLT_MAX) && errno == ERANGE))
		fprintf(stderr, "Could not convert '%s' to float and leftover string is: '%s'\n", s, temp);

	return val;
}

static char* char_to_str(char c)
{
	char* s = (char*)malloc(2 * sizeof(char));
	s[0] = c;
	s[1] = '\0';

	return s;
}

static char* int_to_str(int64_t i)
{
	// A 64-bit integer can have up to 19 digits + sign + null terminator
	char* s = (char*)malloc(21 * sizeof(char));
	if (!s)
		return nullptr;

	snprintf(s, 21, "%" PRId64, i);
	return s;
}

static char* uint_to_str(uint64_t i)
{
	// A 64-bit unsigned integer can have up to 20 digits + null terminator
	char* s = (char*)malloc(21 * sizeof(char));
	if (!s)
		return nullptr;

	snprintf(s, 21, "%" PRIu64, i);
	return s;
}

static char* float_to_str(float f)
{
	int size = 65;
	char* s = (char*)malloc(size);

	snprintf(s, size, "%f", f);
	return s;
}

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes, bool is_global, char* buf)
{
	if (!InterpreterScope::global_scope)
		InterpreterScope::global_scope = &scope;

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
			s.emplace(-pop(s, scope).as.i);
			break;
		case BytecodeType_NEGATIVE_F:
			s.emplace(-pop(s, scope).as.d);
			break;

		case BytecodeType_NOT_I:
			s.emplace((int64_t)!pop(s, scope).as.i);
			break;
		case BytecodeType_NOT_F:
			s.emplace((int64_t)!pop(s, scope).as.d);
			break;

		case BytecodeType_ADD_I:
			s.emplace(pop(s, scope).as.i + pop(s, scope).as.i);
			break;
		case BytecodeType_ADD_F:
			s.emplace(pop(s, scope).as.d + pop(s, scope).as.d);
			break;
		case BytecodeType_ADD_S: {
			auto s1 = pop(s, scope).as.s;
			auto s2 = pop(s, scope).as.s;

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
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(-s1 + s2);
			break;
		}
		case BytecodeType_SUB_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace(-s1 + s2);
			break;
		}

		case BytecodeType_MULT_I: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(s1 * s2);
			break;
		}
		case BytecodeType_MULT_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace(s1 * s2);
			break;
		}

		case BytecodeType_DIV_I: {
			auto s2 = pop(s, scope);
			s.emplace(pop(s, scope).as.i / s2.as.i);
			break;
		}
		case BytecodeType_DIV_F: {
			auto s2 = pop(s, scope);
			s.emplace(pop(s, scope).as.d / s2.as.d);
			break;
		}
		case ByteType_MOD: {
			auto s2 = pop(s, scope);
			s.emplace(pop(s, scope).as.i % s2.as.i);
			break;
		}

		// stack values are in opposite order, so we switch signs to account for that
		case BytecodeType_LESSER_I: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(int64_t(s1 > s2));
			break;
		}
		case BytecodeType_LESSER_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace(int64_t(s1 > s2));
			break;
		}
		case BytecodeType_LESSER_S: {
			auto s1 = pop(s, scope).as.s;
			auto s2 = pop(s, scope).as.s;
			s.emplace(int64_t(strcmp(s1, s2) > 0));
			break;
		}

		case BytecodeType_GREATER_I: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(int64_t(s1 < s2));
			break;
		}
		case BytecodeType_GREATER_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace(int64_t(s1 < s2));
			break;
		}
		case BytecodeType_GREATER_S: {
			auto s1 = pop(s, scope).as.s;
			auto s2 = pop(s, scope).as.s;
			s.emplace(int64_t(strcmp(s1, s2) < 0));
			break;
		}

		case BytecodeType_LESSER_EQUALS_I: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace((int64_t)(s1 >= s2));
			break;
		}
		case BytecodeType_LESSER_EQUALS_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace((int64_t)(s1 >= s2));
			break;
		}
		case BytecodeType_LESSER_EQUALS_S: {
			auto s1 = pop(s, scope).as.s;
			auto s2 = pop(s, scope).as.s;
			s.emplace((int64_t)(strcmp(s1, s2) >= 0));
			break;
		}

		case BytecodeType_GREATER_EQUALS_I: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace((int64_t)(s1 <= s2));
			break;
		}
		case BytecodeType_GREATER_EQUALS_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace((int64_t)(s1 <= s2));
			break;
		}
		case BytecodeType_GREATER_EQUALS_S: {
			auto s1 = pop(s, scope).as.s;
			auto s2 = pop(s, scope).as.s;
			s.emplace((int64_t)(strcmp(s1, s2) <= 0));
			break;
		}

		case BytecodeType_EQUALS_I: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(int64_t(s1 == s2));
			break;
		}
		case BytecodeType_EQUALS_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace(int64_t(s1 == s2));
			break;
		}
		case BytecodeType_EQUALS_S: {
			auto s1 = pop(s, scope).as.s;
			auto s2 = pop(s, scope).as.s;
			s.emplace(int64_t(!strcmp(s1, s2)));
			break;
		}

		case BytecodeType_NOT_EQUALS_I: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(int64_t(s1 != s2));
			break;
		}
		case BytecodeType_NOT_EQUALS_F: {
			auto s1 = pop(s, scope).as.d;
			auto s2 = pop(s, scope).as.d;
			s.emplace(int64_t(s1 != s2));
			break;
		}
		case BytecodeType_NOT_EQUALS_S: {
			auto s1 = pop(s, scope).as.s;
			auto s2 = pop(s, scope).as.s;
			s.emplace((int64_t)strcmp(s1, s2));
			break;
		}

		case BytecodeType_AND: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(int64_t(s1 && s2));
			break;
		}
		case BytecodeType_OR: {
			auto s1 = pop(s, scope).as.i;
			auto s2 = pop(s, scope).as.i;
			s.emplace(int64_t(s1 || s2));
			break;
		}

		case BytecodeType_INDEX_S: push_subscript(s, true, scope); break;
		case BytecodeType_INDEX_A: push_subscript(s, false, scope); break;


		case ByteType_DUP: {
			s.emplace(s.top());
			break;
		}

		case ByteType_LOAD: {
			night::id_t id = pop(s, scope).as.ui;
			s.emplace(&scope.get_variable(id), true);

			break;
		}

		case ByteType_STORE: {
			night::id_t id = pop(s, scope).as.ui;
			scope.set_variable(id, pop(s, scope));

			break;
		}
		
		case ByteType_STORE_INPLACE: {
			intpr::Value val = pop(s, scope);
			intpr::Value* var = pop(s, scope, true).as.var;
			*var = val;
			s.emplace(var, true);
			break;
		}

		case BytecodeType_LOAD_ELEM: {
			uint64_t id = pop(s, scope).as.ui;
			uint64_t num = pop(s, scope).as.ui;
			intpr::Value* val = &scope.get_variable(id);
			while (num--)
			{
				auto i = pop(s, scope).as.i;
				val = &val->as.a.data[i];
			}
			s.push(*val);
			break;
		}

		case BytecodeType_ALLOCATE_STR: push_str(s, scope); break;
		case BytecodeType_ALLOCATE_ARR: push_arr(s, scope); break;
		case BytecodeType_ALLOCATE_ARR_AND_FILL: push_arr_and_fill(s, scope); break;

		case BytecodeType_STORE_INDEX_A: {
			auto id = pop(s, scope).as.i;
			auto expr = pop(s, scope);
			intpr::Value* val = &scope.get_variable(id);
			while (!s.empty())
			{
				auto i = pop(s, scope).as.i;
				val = &val->as.a.data[i];
			}
			*val = expr;
			break;
		}

		case BytecodeType_STORE_INDEX_S: {
			auto id = pop(s, scope).as.i;
			auto expr = pop(s, scope);
			scope.get_variable(id).as.s[pop(s, scope).as.i] = (char)expr.as.i;
			break;
		}

		case BytecodeType_JUMP_IF_FALSE: {
			auto offset = pop(s, scope).as.i;
			if (!pop(s, scope).as.i)
				std::advance(it, offset);
			break;
		}

		case BytecodeType_JUMP:
			std::advance(it, pop(s, scope).as.i);
			break;
		case ByteType_JUMP_N:
			std::advance(it, -pop(s, scope).as.i);
			freeze = true;
			break;

		case BytecodeType_RETURN: {
			if (s.empty())
				return std::nullopt;

			return pop(s, scope);
		}

		case BytecodeType_CALL: {
			night::id_t id = pop(s, scope).as.i;

			switch (id)
			{
			case PredefinedFunctions::PRINT_BOOL:
				if (!buf)
					printf(pop(s, scope).as.i ? "true" : "false");
				else {
					const char* bool_str = pop(s, scope).as.i ? "true" : "false";
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len; // Assume buffer is at least 1024 bytes
					snprintf(buf + current_len, remaining, "%s", bool_str);
				}
				break;
			case PredefinedFunctions::PRINT_CHAR:
				if (!buf)
					printf("%c", (char)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%c", (char)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_INT8:
				if (!buf)
					printf("%" PRId8, (int8_t)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRId8, (int8_t)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_INT16:
				if (!buf)
					printf("%" PRId16, (int16_t)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRId16, (int16_t)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_INT32:
				if (!buf)
					printf("%" PRId32, (int32_t)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRId32, (int32_t)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_INT64:
				if (!buf)
					printf("%" PRId64, pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRId64, pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_uINT8:
				if (!buf)
					printf("%" PRIu8, (uint8_t)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRIu8, (uint8_t)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_uINT16:
				if (!buf)
					printf("%" PRIu16, (uint16_t)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRIu16, (uint16_t)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_uINT32:
				if (!buf)
					printf("%" PRIu32, (uint32_t)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRIu32, (uint32_t)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_uINT64:
				if (!buf)
					printf("%" PRIu64, (uint64_t)pop(s, scope).as.i);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%" PRIu64, (uint64_t)pop(s, scope).as.i);
				}
				break;
			case PredefinedFunctions::PRINT_FLOAT:
				if (!buf)
					printf("%.17gf", pop(s, scope).as.d);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%.17gf", pop(s, scope).as.d);
				}
				break;
			case PredefinedFunctions::PRINT_STR:
				if (!buf)
					printf("%s", pop(s, scope).as.s);
				else {
					size_t current_len = strlen(buf);
					size_t remaining = 1024 - current_len;
					snprintf(buf + current_len, remaining, "%s", pop(s, scope).as.s);
				}
				break;

			case PredefinedFunctions::INPUT:
				s.emplace(interpret_predefined_input());
				break;

			case PredefinedFunctions::INT8_TO_CHAR:
				s.emplace((int64_t)(char)(int8_t)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::INT16_TO_CHAR:
				s.emplace((int64_t)(char)(int16_t)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::INT32_TO_CHAR:
				s.emplace((int64_t)(char)(int32_t)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::INT64_TO_CHAR:
				s.emplace((int64_t)(char)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::uINT8_TO_CHAR:
				s.emplace((int64_t)(char)(uint8_t)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::uINT16_TO_CHAR:
				s.emplace((int64_t)(char)(uint16_t)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::uINT32_TO_CHAR:
				s.emplace((int64_t)(char)(uint32_t)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::uINT64_TO_CHAR:
				s.emplace((int64_t)(char)(uint64_t)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::STR_TO_CHAR: {
				char* str = pop(s, scope).as.s;
				if (strlen(str) != 1)
					throw night::error::get().create_runtime_error("Could not convert string to char.");

				s.emplace((int64_t)str[0]);
				break;
			}

			case PredefinedFunctions::BOOL_TO_INT8:
			case PredefinedFunctions::BOOL_TO_INT16:
			case PredefinedFunctions::BOOL_TO_INT32:
			case PredefinedFunctions::BOOL_TO_INT64:
				s.emplace(pop(s, scope).as.i);
				break;
			case PredefinedFunctions::BOOL_TO_uINT8:
			case PredefinedFunctions::BOOL_TO_uINT16:
			case PredefinedFunctions::BOOL_TO_uINT32:
			case PredefinedFunctions::BOOL_TO_uINT64:
				s.emplace((uint64_t)pop(s, scope).as.i);
				break;

			case PredefinedFunctions::CHAR_TO_INT8:
			case PredefinedFunctions::CHAR_TO_INT16:
			case PredefinedFunctions::CHAR_TO_INT32:
			case PredefinedFunctions::CHAR_TO_INT64:
				s.emplace(pop(s, scope).as.i);
				break;
			case PredefinedFunctions::CHAR_TO_uINT8:
			case PredefinedFunctions::CHAR_TO_uINT16:
			case PredefinedFunctions::CHAR_TO_uINT32:
			case PredefinedFunctions::CHAR_TO_uINT64:
				s.emplace((uint64_t)pop(s, scope).as.i);
				break;

			case PredefinedFunctions::FLOAT_TO_INT8:
			case PredefinedFunctions::FLOAT_TO_INT16:
			case PredefinedFunctions::FLOAT_TO_INT32:
			case PredefinedFunctions::FLOAT_TO_INT64:
				s.emplace((int64_t)pop(s, scope).as.d);
				break;
			case PredefinedFunctions::FLOAT_TO_uINT8:
			case PredefinedFunctions::FLOAT_TO_uINT16:
			case PredefinedFunctions::FLOAT_TO_uINT32:
			case PredefinedFunctions::FLOAT_TO_uINT64:
				s.emplace((uint64_t)pop(s, scope).as.d);
				break;

			case PredefinedFunctions::STR_TO_INT8:
			case PredefinedFunctions::STR_TO_INT16:
			case PredefinedFunctions::STR_TO_INT32:
			case PredefinedFunctions::STR_TO_INT64:
				s.emplace(str_to_int(pop(s, scope).as.s));
				break;
			case PredefinedFunctions::STR_TO_uINT8:
			case PredefinedFunctions::STR_TO_uINT16:
			case PredefinedFunctions::STR_TO_uINT32:
			case PredefinedFunctions::STR_TO_uINT64:
				s.emplace((uint64_t)str_to_int(pop(s, scope).as.s));
				break;

			case PredefinedFunctions::BOOL_TO_FLOAT:
				s.emplace(pop(s, scope).as.i ? 1.0f : 0.0f);
				break;
			case PredefinedFunctions::CHAR_TO_FLOAT:
				s.emplace((double)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::INT8_TO_FLOAT:
			case PredefinedFunctions::INT16_TO_FLOAT:
			case PredefinedFunctions::INT32_TO_FLOAT:
			case PredefinedFunctions::INT64_TO_FLOAT:
				s.emplace((double)pop(s, scope).as.i);
				break;
			case PredefinedFunctions::uINT8_TO_FLOAT:
			case PredefinedFunctions::uINT16_TO_FLOAT:
			case PredefinedFunctions::uINT32_TO_FLOAT:
			case PredefinedFunctions::uINT64_TO_FLOAT:
				s.emplace((double)pop(s, scope).as.ui);
				break;

			case PredefinedFunctions::STR_TO_FLOAT:
				s.emplace(str_to_float(pop(s, scope).as.s));
				break;

			case PredefinedFunctions::CHAR_TO_STR:
				s.emplace(char_to_str((char)pop(s, scope).as.i));
				break;
			case PredefinedFunctions::INT8_TO_STR:
			case PredefinedFunctions::INT16_TO_STR:
			case PredefinedFunctions::INT32_TO_STR:
			case PredefinedFunctions::INT64_TO_STR:
				s.emplace(int_to_str(pop(s, scope).as.i));
				break;
			case PredefinedFunctions::uINT8_TO_STR:
			case PredefinedFunctions::uINT16_TO_STR:
			case PredefinedFunctions::uINT32_TO_STR:
			case PredefinedFunctions::uINT64_TO_STR:
				s.emplace(uint_to_str(pop(s, scope).as.ui));
				break;
			case PredefinedFunctions::FLOAT_TO_STR:
				s.emplace(float_to_str((float)pop(s, scope).as.d));
				break;

			case PredefinedFunctions::LEN:
				s.emplace((int64_t)strlen(pop(s, scope).as.s));
				break;
			default: {
				// Functions' only allowed parent scope is the global scope
				InterpreterScope func_scope(InterpreterScope::global_scope);

				for (int i = scope.funcs[id].param_ids.size() - 1; i >= 0; --i)
					func_scope.set_variable(InterpreterScope::funcs[id].param_ids[i], pop(s, scope));

				auto rtn_value = interpret_bytecodes(func_scope, InterpreterScope::funcs[id].codes, false, buf);
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

char* interpret_predefined_input()
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
	return buf;
}

void push_str(std::stack<intpr::Value>& s, InterpreterScope& scope)
{
	uint64_t size = pop(s, scope).as.ui;
	char* arr = (char*)malloc((size + 1) * sizeof(char));

	for (uint64_t i = size; i > 0; --i)
		arr[i - 1] = (char)pop(s, scope).as.i;

	arr[size] = '\0';

	s.push(arr);
}

void push_arr(std::stack<intpr::Value>& s, InterpreterScope& scope)
{
	uint64_t size = pop(s, scope).as.ui;
	intpr::Value arr;
	arr.as.a.size = size;
	arr.as.a.data = new intpr::Value[size];

	for (uint64_t i = size; i > 0; --i)
		arr.as.a.data[i - 1] = pop(s, scope);

	s.push(arr);
}

void push_arr_and_fill(std::stack<intpr::Value>& s, InterpreterScope& scope)
{
	uint64_t dimensions = pop(s, scope).as.ui;
	std::vector<int> sizes(dimensions);

	for (uint64_t i = dimensions; i > 0; --i)
		sizes[i - 1] = (int)pop(s, scope).as.i;
	
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

void push_subscript(std::stack<intpr::Value>& s, bool is_string, InterpreterScope& scope)
{
	auto container = pop(s, scope, true);
	auto index = pop(s, scope);

	if (container.is_var)
	{
		if (is_string)
			s.emplace((int64_t)container.as.var->as.s[index.as.i]);
		else
			s.emplace(&container.as.var->as.a.data[index.as.i], true);
	}
	else
	{
		if (is_string)
			s.emplace((int64_t)container.as.s[index.as.i]);
		else
			s.emplace(container.as.a.data[index.as.i]);
	}
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

intpr::Value pop(std::stack<intpr::Value>& s, InterpreterScope& scope, bool want_var)
{
	assert(!s.empty());

	auto val = s.top();
	s.pop();

	if (!want_var && val.is_var)
		return *val.as.var;
	return val;
}
