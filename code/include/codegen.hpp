#pragma once

// lexer
//   start: first token of statement
//   end: last token of statement
// bytecode for functions are not saved in a .bnight file with the others,
// instead it is stored in a Scope
void generate_codes_func(func_container& funcs, Lexer& lexer, Scope& scope);