#pragma once

#include <stdio.h>

enum PredefinedFunctions
{
	PRINT_BOOL,
	PRINT_CHAR,
	PRINT_INT,
	PRINT_FLOAT,
	PRINT_STR,
	
	INPUT,
	
	INT_TO_CHAR,
	STR_TO_CHAR,

	BOOL_TO_INT,
	CHAR_TO_INT,
	FLOAT_TO_INT,
	STR_TO_INT,

	BOOL_TO_FLOAT,
	CHAR_TO_FLOAT,
	INT_TO_FLOAT,
	STR_TO_FLOAT,

	CHAR_TO_STR,
	INT_TO_STR,
	FLOAT_TO_STR,

	LEN,
};

int bool_to_int(bool b);

int char_to_int(char c);

int float_to_int(double d);

long long str_to_int(char* s);

float bool_to_float(bool b);

float char_to_float(char c);

float int_to_float(int i);

float str_to_float(char* s);

char* char_to_str(char c);

char* int_to_str(int i);

char* float_to_str(float f);

int len(char* s);
