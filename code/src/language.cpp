#include "language.hpp"
#include "common/error.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>

int bool_to_int(bool b)
{
	return (int)b;
}

int char_to_int(char c)
{
	return (int)c;
}

int float_to_int(double d)
{
	return (int)d;
}

long long str_to_int(char* s)
{
	assert(s);

	errno = 0;
	char* temp;
	long long val = strtoll(s, &temp, 10);

	if (temp == s || *temp != '\0' ||
		((val == LLONG_MIN || val == LLONG_MAX) && errno == ERANGE))
		fprintf(stderr, "Could not convert '%s' to long long and leftover string is: '%s'\n", s, temp);

	return val;
}

float bool_to_float(bool b)
{
	return (float)b;
}

float char_to_float(char c)
{
	return (float)c;
}

float int_to_float(int i)
{
	return (float)i;
}

float str_to_float(char* s)
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

char* char_to_str(char c)
{
	char* s = (char*)malloc(2 * sizeof(char));
	s[0] = c;
	s[1] = '\0';

	return s;
}

char* int_to_str(int i)
{
	char* s = (char*)malloc(32 * sizeof(char));

	sprintf(s, "%d", i);
	return s;
}

char* float_to_str(float f)
{
	int size = 65;
	char* s = (char*)malloc(size);

	sprintf(s, "%f", f);
	return s;
}

int len(char* s)
{
	assert(s);

	return strlen(s);
}