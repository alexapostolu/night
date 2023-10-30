#include "code_gen.hpp"

#include "ast/statement.hpp"
#include "bytecode.hpp"

bytecodes_t code_gen(std::vector<stmt_p>& block)
{
	ParserScope global_scope;
	bytecodes_t codes;

	for (auto& ast : block)
		ast->check(global_scope);

	if (night::error::get().has_minor_errors())
		throw night::error::get();

	for (auto& ast : block)
		ast->optimize(global_scope);

	if (night::error::get().has_minor_errors())
		throw night::error::get();

	for (auto const& ast : block)
	{
		auto ast_codes = ast->generate_codes();
		codes.insert(std::end(codes), std::begin(ast_codes), std::end(ast_codes));
	}

	return codes;
}