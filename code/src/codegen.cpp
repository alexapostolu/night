#include "codegen.hpp"

void generate_codes_var(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	std::string var_name = lexer.curr().str;

	lexer.eat();

	if (lexer.curr().is_type())
		generate_codes_var_init(codes, lexer, scope, var_name);
	else if (lexer.curr().type == TokenType::ASSIGN)
		generate_codes_var_assign(codes, lexer, scope, var_name, true);
	else
		throw NIGHT_CREATE_FATAL("expected variable type or assignment after variable name '" + var_name + "'");
}

void generate_codes_if(bytecodes_t& codes, Lexer& lexer, Scope& scope, bool is_elif)
{
	assert(lexer.curr().type == TokenType::IF);

	// parse condition

	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_toks_expr(lexer, scope,
		"expected expression after opening bracket in if statement");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && *cond_type == ValueType::OBJECT)
		NIGHT_CREATE_MINOR("condition of type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	generate_codes_expr(codes, cond_expr);

	lexer.expect(TokenType::CLOSE_BRACKET);

	// parse statements

	bool curly_enclosed = false;
	auto stmt_codes = parse_stmts(lexer, scope, &curly_enclosed);

	if (!curly_enclosed && stmt_codes.empty())
		NIGHT_CREATE_MINOR("if statement missing body");

	codes.push_back((bytecode_t)(is_elif ? BytecodeType::ELIF : BytecodeType::IF));
	codes.push_back((bytecode_t)stmt_codes.size());
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

void generate_codes_else(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::ELSE);

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back((bytecode_t)BytecodeType::ELSE);
	codes.push_back((bytecode_t)stmt_codes.size());
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

void generate_codes_for(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::FOR);

	lexer.expect(TokenType::OPEN_BRACKET);

	bytecodes_t codes;

	// variable initialization

	std::string var_name = lexer.eat().str;
	type_check::var_undefined(lexer, scope, var_name);

	if (!lexer.eat().is_type())
		throw NIGHT_CREATE_FATAL("expected variable type after variable name in for loop initialization");

	generate_codes_var_init(codes, lexer, scope, var_name);

	// condition

	lexer.eat();

	auto cond_expr = parse_toks_expr(lexer, scope,
		"expected condition after variable initialization in for loop");

	if (lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("expected semicolon after condition in for loop");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && *cond_type == ValueType::OBJECT)
		NIGHT_CREATE_MINOR("for loop condition is type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	generate_codes_expr(codes, cond_expr);

	// increment

	auto inc_name = lexer.expect(TokenType::VARIABLE);
	type_check::var_defined(lexer, scope, var_name);

	lexer.expect(TokenType::ASSIGN);

	generate_codes_var_assign(codes, lexer, scope, var_name, false);

	if (lexer.curr().type != TokenType::CLOSE_BRACKET)
		throw NIGHT_CREATE_FATAL("expected closing bracket after for loop increment");

	// statements

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back((bytecode_t)BytecodeType::FOR);
	codes.push_back((bytecode_t)stmt_codes.size());
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

void generate_codes_while(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::WHILE);

	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_toks_expr(lexer, scope,
		"expected expression after opening bracket in while loop");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && *cond_type == ValueType::OBJECT)
		NIGHT_CREATE_MINOR("while loop condition is type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	if (lexer.curr().type != TokenType::CLOSE_BRACKET)
		throw NIGHT_CREATE_FATAL("expected closing bracket after while loop condition");

	// statements

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back((bytecode_t)BytecodeType::WHILE);
	codes.push_back((bytecode_t)stmt_codes.size());

	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

void generate_codes_func(func_container& funcs, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::DEF);

	auto const func_name = lexer.expect(TokenType::VARIABLE).str;

	scope.funcs[func_name] = {};

	lexer.expect(TokenType::OPEN_BRACKET);

	// parameters

	Scope func_scope;

	scope.funcs[func_name].param_types = parse_params(lexer, scope);

	// return values

	if (!lexer.eat().is_type())
		throw NIGHT_CREATE_FATAL("expected type after parameters in function definition of '" + func_name + "'");

	switch (lexer.curr().type)
	{
	case TokenType::BOOL_LIT:
		scope.funcs[func_name].rtn_type = ValueType::BOOL;
	case TokenType::CHAR_LIT:
		scope.funcs[func_name].rtn_type = ValueType::CHAR;
	case TokenType::INT_LIT:
		scope.funcs[func_name].rtn_type = ValueType::INT;
	}

	// body

	funcs[func_name] = {};
	Scope func_scope;
	for (auto var : scope.funcs[func_name].param_types)

		bool curly_enclosed = false;
	auto stmt_codes = parse_stmts(lexer, scope, funcs, &curly_enclosed);
	if (!curly_enclosed)
		NIGHT_CREATE_MINOR("function body must be enclosed by curly brackets");

	funcs[func_name].codes.insert(std::end(funcs[func_name].codes), std::begin(stmt_codes), std::end(stmt_codes));

	if (scope.vars.contains(func_name))
		NIGHT_CREATE_MINOR("function '" + func_name + "' can not have the same name as a variable");
	if (scope.funcs.contains(func_name))
		NIGHT_CREATE_MINOR("function '" + func_name + "' is already defined");

	funcs[func_name].params = {};
}
