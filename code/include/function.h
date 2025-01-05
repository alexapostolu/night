#pragma once

#include "bytecode.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct function_t {
	uint64_t* param_ids;
	size_t param_count;

	byte_t* bytes;
	size_t bytes_count;
} function_t;

#ifdef __cplusplus
}
#endif
