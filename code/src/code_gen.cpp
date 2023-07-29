#include "code_gen.hpp"
#include "bytecode.hpp"
#include "ast/ast.hpp"

#include <stack>

bytecodes_t code_gen(AST_Block const& block)
{
	std::stack<bytecodes_t::iterator> jumps;
	bytecodes_t codes;

	for (auto const& ast : block)
	{
		auto ast_codes = ast->generate_codes();
		codes.insert(std::end(codes), std::begin(ast_codes), std::end(ast_codes));
	}
}