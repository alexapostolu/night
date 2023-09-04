#include "code_gen.hpp"
#include "bytecode.hpp"
#include "ast/ast.hpp"

bytecodes_t code_gen(AST_Block const& block)
{
	ParserScope global_scope;
	bytecodes_t codes;

	for (auto const& ast : block)
		ast->check(global_scope);

	if (night::error::get().has_minor_errors())
		throw night::error::get();

	for (auto const& ast : block)
	{
		auto ast_codes = ast->generate_codes();
		codes.insert(std::end(codes), std::begin(ast_codes), std::end(ast_codes));
	}

	return codes;
}