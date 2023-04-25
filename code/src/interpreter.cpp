#include "interpreter.hpp"

#include <assert.h>
#include <stdexcept>

// debugging with asserts
#define assert_assignment() \
	assert(scope.vars.contains(code->val) \
		&& "variable definitions should be checked in parser"); \
	assert(!s.empty() \
		&& "the stack should not be empty when assigning, should be checked in parser");

Interpreter::Interpreter(bytecodes_t const& bytecodes)
{
	interpret_bytecodes(bytecodes);
}

void Interpreter::interpret_bytecodes(bytecodes_t const& codes)
{
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
			scope.vars[code->val] = { ValueType::BOOL, pop(s) };
			break;
		case BytecodeType::CHAR_ASSIGN:
			assert_assignment();
			scope.vars[code->val] = { ValueType::CHAR, pop(s) };
			break;
		case BytecodeType::INT_ASSIGN:
			assert_assignment();
			scope.vars[code->val] = { ValueType::INT, pop(s) };
			break;

		case BytecodeType::ADD_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val += pop(s);
			break;
		case BytecodeType::SUB_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val -= pop(s);
			break;
		case BytecodeType::MULT_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val *= pop(s);
			break;
		case BytecodeType::DIV_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val /= pop(s);
			break;

		case BytecodeType::IF:
		case BytecodeType::ELIF:
			if (code->val)
			{
				interpret_bytecodes(bytecodes_t(++code, std::next(code, code->val + 1)));
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
				interpret_bytecodes(bytecodes_t(++code, std::next(code, code->val + 1)));
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

	return flag ? scope.vars[val].val : val;
}