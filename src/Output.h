#pragma once

#include <stdio.h>
#include "lib/string.h"

namespace {

night::string output;

}

void StoreOutput(const night::string& text)
{
	output += text;
}

void PrintOutput()
{
	for (int a = 0; a < output.length() - 1; ++a)
	{
		if (output[a] == '\\' && output[a + 1] == 'n')
		{
			output[a] = '\n';
			output.remove(a + 1);

			a--;
		}
	}

	printf("%s", output.cstr());
}