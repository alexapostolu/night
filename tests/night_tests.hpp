#pragma once

#include "assert.h"
#include <iostream>

#define night_assert(msg, cond)	std::clog << " . " << (#cond); \
								if (!(cond)) { std::clog << " . . assertion failed\n         " << msg << "\n"; } \
								else		 { std::clog << " . . assertion passed\n"; }