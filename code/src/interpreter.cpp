#include "interpreter.hpp"

#include <assert.h>
#include <stdexcept>

Interpreter::Interpreter(bytecodes_t const& bytecodes)
{
	parse_bytecodes(bytecodes);
}

void Interpreter::parse_bytecodes(bytecodes_t const& codes)
{
	bool skip_cond_stmt = false;
	expr_stack s;

	for (auto code = std::begin(codes); code != std::end(codes); ++code)
	{
		switch (code->type)
		{
		case BytecodeType::CONSTANT: s.push({ false, code->val }); break;
		case BytecodeType::VARIABLE: s.push({ true,  code->val }); break;

		case BytecodeType::NOT:  s.push({ false, !pop(s)		 }); break;
		case BytecodeType::ADD:  s.push({ false, pop(s) + pop(s) }); break;
		case BytecodeType::SUB:  s.push({ false, pop(s) - pop(s) }); break;
		case BytecodeType::MULT: s.push({ false, pop(s) * pop(s) }); break;
		case BytecodeType::DIV:  s.push({ false, pop(s) / pop(s) }); break;

		case BytecodeType::BOOL_ASSIGN:
			assert_assignment();
			scope.vars[code->val] = pop(s);
			break;

		case BytecodeType::ADD_ASSIGN:
			assert_assignment();
			scope.vars[code->val] += pop(s);
			break;

		case BytecodeType::SUB_ASSIGN:
			assert_assignment();
			scope.vars[code->val] -= pop(s);
			break;

		case BytecodeType::MULT_ASSIGN:
			assert_assignment();
			scope.vars[code->val] *= pop(s);
			break;

		case BytecodeType::DIV_ASSIGN:
			assert_assignment();
			scope.vars[code->val] /= pop(s);
			break;

		case BytecodeType::IF:
		case BytecodeType::ELIF:
			if (code->val)
			{
				parse_bytecodes(bytecodes_t(++code, std::next(code, code->val + 1)));
				std::advance(code, code->val + 1);

				while (code != std::end(codes) && (code->type == BytecodeType::ELIF || code->type == BytecodeType::ELSE))
					std::advance(code, code->val + 1);
				--code;
			}
			else
			{
				std::advance(code, code->val);
			}
			break;
		case BytecodeType::ELSE:
			if (code->val)
				parse_bytecodes(bytecodes_t(++code, std::next(code, code->val + 1)));
			else
				std::advance(code, code->val);
			break;

		default:
			throw std::runtime_error("Interpreter::parse_bytecodes unhandled case " + bytecode_to_str(code->type));
		}
	}
}

int Interpreter::pop(expr_stack& s)
{
	assert(!s.empty() && "u momma gay");

	auto const& [flag, val] = s.top();
	s.pop();

	return flag ? scope.vars[val] : val;
}