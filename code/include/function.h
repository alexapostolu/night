#pragma once

#include "bytecode.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct function_t {
	size_t param_count;
	uint64_t* param_ids;

	size_t bytes_count;
	byte_t* bytes;
} function_t;

#ifdef __cplusplus
}
#endif
