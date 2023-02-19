#pragma once

#include "assert.h"
#include <iostream>

#include "bytecode.hpp"

#define night_assert(msg, cond) if (!(cond)) { std::clog << " . . assertion failed\n         " << msg << "\n"; } \
								else { std::clog << " . . assertion passed\n"; }

#define night_assert_expect_bytecode(i, _type) night_assert("expected " + bytecode_to_str(_type), codes[i]->type == _type);