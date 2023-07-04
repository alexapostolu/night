#pragma once

#include <stdint.h>
#include <string.h>
#include "bytecode.hpp"

template<typename T>

void concat_vector(std::vector<T>& vec1, std::vector<T> const& vec2)
{
	vec1.insert(std::end(vec1), std::begin(vec2), std::end(vec2));
}

int64_t bytecode_to_snum(bytecode_size arr[])
{
	int64_t value;
	memcpy_s(&value, sizeof(value), code->val, sizeof(value2));

	return value;
}

int64_t bytecode_to_unum()
{
	uint8_t value;
	memcpy_s(&value, sizeof(value), code->val, sizeof(value2));

	return value;
}