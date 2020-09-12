#pragma once

#include "./string.h"
#include "./array.h"

#include "../containers/token.h"

const night::string WHITE = "\033[38;5;253m";
const night::string RED   = "\033[38;5;160m";
const night::string I_RED = "\033[38;5;196m";
const night::string U_RED = "\033[38;5;160;4m";
const night::string BLUE  = "\033[38;5;39m";
const night::string RESET = "\033[0m";

namespace night {

const night::string _missing_token_   { "missing token"   };
const night::string _invalid_token_   { "invalid token"   };
const night::string _undefined_token_ { "undefined token" };
const night::string _redefined_token_ { "redefined token" };

const night::string _invalid_expression_ { "invalid expression" };
const night::string _invalid_variable_   { "invalid variable"   };
const night::string _invalid_statement_  { "invalid statement"  };
const night::string _invalid_function_   { "invalid function"   };
const night::string _invalid_grammar_    { "invalid grammar"    };

} // namespace night

class Error
{
public:
	Error(const night::string& _errorType, const night::array<Token>& _code, int _start, int _end,
		const night::string& _errorMsg)
		: errorType(_errorType), code(_code), start(_start), end(_end), errorMsg(_errorMsg)
	{
		for (int a = 0; a < code.length(); ++a)
		{
			if (code[a].type == TokenType::SYB_VALUE)
				code[a].value = "'"_s + code[a].value + "'";
			else if (code[a].type == TokenType::STR_VALUE)
				code[a].value = "\""_s + code[a].value + "\"";
		}
	}

	night::string what() const
	{
		night::string output;

		output += I_RED + "Error - "_s + RESET;
		output += U_RED + errorType + "\n\n"_s + RESET;

		for (int a = 0; a < code.length(); ++a)
			output += (a >= start && a <= end ? RED : WHITE) + code[a].value + ' ';

		output += '\n';

		for (int a = 0; a < code.length(); ++a)
		{
			for (int b = 0; b < code[a].value.length(); ++b)
				output += RESET + (a < start || a > end ? ' ' : '~');
			output += ((a < start || a > end) || (a == end) ? ' ' : '~');
		}

		output += "\n\n";

		bool inside = false;
		for (int a = 0; a < errorMsg.length(); ++a)
		{
			if (errorMsg[a] == '\'')
			{
				inside = !inside;
				output += WHITE + errorMsg[a] + RESET;
				continue;
			}

			output += (inside ? BLUE : WHITE) + errorMsg[a] + RESET;
		}

		return output;
	}

public:
	night::string errorType;
	night::array<Token> code;
	int start, end;
	night::string errorMsg;
};