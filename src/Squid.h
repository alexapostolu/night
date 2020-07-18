#pragma once

#include <iostream>
#include <string>

constexpr int DYNAMIC_SQUID(int excNum) { return excNum; }

void SquidError(int excNum)
{
	std::cout << "[" << excNum << "] Oops, something went wrong! Please submit an issue at: https://github.com/DynamicSquid/Night \n";
}

class squid
{
private:
	std::string excType;

public:
	squid() {}
	squid(std::string exc)
		: excType(exc) {}

	squid operator()(const std::string&& exc)
	{
		excType = exc;
		return *this;
	}

	std::string what() const
	{
		return "Error - " + excType;
	}
};

squid _undefined_token_;
squid _undefined_data_type_;
squid _undefined_variable_;
squid _variable_redefinition_;

const squid _missing_semicolon_("semicolon is missing");
const squid _missing_open_bracket_("open bracket is missing");
const squid _missing_close_bracket_("close bracket is missing");
const squid _missing_open_curly_("open curly bracket is missing");
const squid _missing_close_curly_("close curly bracket is missing");

const squid _invalid_bit_expr_("bit expression is invalid");
const squid _invalid_syb_expr_("symbol expression is invalid");
const squid _invalid_int_expr_("integer expression is invalid");
const squid _invalid_dec_expr_("decimal expression is invalid");
const squid _invalid_str_expr_("string expression is invalid");

const squid _invalid_print_("print statement is invalid");
const squid _invalid_if_statement_("if statement is invalid");

const squid _invalid_grammar_("language grammar is invalid");