#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "scope.hpp"

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope);

bytecodes_t parse_var(Lexer& lexer, Scope& scope);
bytecodes_t parse_if(Lexer& lexer, Scope& scope);
bytecodes_t parse_for(Lexer& lexer, Scope& scope);
bytecodes_t parse_while(Lexer& lexer, Scope& scope);
bytecodes_t parse_rtn(Lexer& lexer, Scope& scope);