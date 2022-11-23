#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "scope.hpp"

#include <memory>

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope);

std::vector<std::shared_ptr<Bytecode>> parse_stmt_var(Lexer& lexer, Scope& scope);
std::vector<std::shared_ptr<Bytecode>> parse_stmt_if(Lexer& lexer, Scope& scope);
std::vector<std::shared_ptr<Bytecode>> parse_stmt_for(Lexer& lexer, Scope& scope);
std::vector<std::shared_ptr<Bytecode>> parse_stmt_while(Lexer& lexer, Scope& scope);
std::vector<std::shared_ptr<Bytecode>> parse_stmt_rtn(Lexer& lexer, Scope& scope);
std::vector<std::shared_ptr<Bytecode>> parse_stmt_var(Lexer& lexer, Scope& scope);