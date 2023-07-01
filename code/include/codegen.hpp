#pragma once

void generate_codes_var(bytecodes_t& codes, Lexer& lexer, Scope& scope);
void generate_codes_if(bytecodes_t& codes, Lexer& lexer, Scope& scope, bool is_elif);
void generate_codes_else(bytecodes_t& codes, Lexer& lexer, Scope& scope);
void generate_codes_for(bytecodes_t& codes, Lexer& lexer, Scope& scope);
void generate_codes_while(bytecodes_t& codes, Lexer& lexer, Scope& scope);
void generate_codes_return(bytecodes_t& codes, Lexer& lexer, Scope& scope);

// lexer
//   start: first token of statement
//   end: last token of statement
// bytecode for functions are not saved in a .bnight file with the others,
// instead it is stored in a Scope
void generate_codes_func(func_container& funcs, Lexer& lexer, Scope& scope);