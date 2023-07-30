#pragma once

#include <stdint.h>
#include <string.h>
#include "bytecode.hpp"

template<typename T>


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