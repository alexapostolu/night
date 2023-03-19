#pragma once

#define night_assert_expect_bytecode(type) night_assert("expected " + bytecode_to_str(_type), codes[i]->type == _type);



void test_parser();
void test_parse_var();
void test_parse_if();
void test_parse_else();