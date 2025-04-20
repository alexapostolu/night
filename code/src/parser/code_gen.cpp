#include "parser/code_gen.hpp"
#include "parser/ast/statement.hpp"
#include "common/bytecode.hpp"

bytecodes_t code_gen(std::vector<stmt_p>& block)
{
	StatementScope global_scope;
	bytecodes_t codes;

	for (auto& ast : block)
		ast->check(global_scope);

	if (night::error::get().has_minor_errors())
		throw night::error::get();

	for (auto i = 0; i < block.size(); ++i)
	{
		bool keep = block[i]->optimize(global_scope);

		if (!keep)
		{
			block.erase(block.begin() + i);
			--i;
		}
	}

	if (night::error::get().has_minor_errors())
		throw night::error::get();

	for (auto const& ast : block)
	{
		auto ast_codes = ast->generate_codes();
		codes.insert(std::end(codes), std::begin(ast_codes), std::end(ast_codes));
	}

	return codes;
}